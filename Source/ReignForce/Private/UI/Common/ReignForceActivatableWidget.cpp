// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/ReignForceActivatableWidget.h"
#include "Framework/Application/SlateApplication.h"

#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/SizeBox.h"

#include "UI/Common/MenuLayoutUtils.h"
#include "UI/Settings/GameSettingsWidget.h"
#include "Utils/InputDeviceUtils.h"


namespace
{
    constexpr float MIN_SCREEN_LIMITED_HEIGHT = 120;
}

UReignForceActivatableWidget::UReignForceActivatableWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bAutoActivate = true;
    bIsBackHandler = true;
    bRestoreLostFocus = true;

    SettingsMenuClass = UGameSettingsWidget::StaticClass();
}

void UReignForceActivatableWidget::RegisterFocusableButton(UButton* Button)
{
    if (!IsValid(Button)) return;
    const bool bRegistered = FocusableButtons.ContainsByPredicate([&] (const FFocusableButtonState& State) { return State.Button == Button; });
    if (bRegistered) return;

    FFocusableButtonState& State = FocusableButtons.AddDefaulted_GetRef();
    State.Button = Button;
    State.DefaultStyle = Button->GetStyle();
    // Plain text tooltips only repeat the button labels in the engine default look
    Button->SetToolTipText(FText::GetEmpty());

    // The focus frame is the only highlight: the hovered brush (white in the menus) would also show on gamepad
    // focus, since the CommonUI virtual cursor hovers the focused button, and hide the white label
    FButtonStyle Style = State.DefaultStyle;
    Style.Hovered = Style.Normal;
    Button->SetStyle(Style);
    State.Prompt.Attach(WidgetTree, Button, State.DefaultStyle.NormalPadding);

    Button->OnHovered.AddDynamic(this, &UReignForceActivatableWidget::HandleButtonHovered);
}

void UReignForceActivatableWidget::SetBackActionButton(UButton* Button)
{
    BackActionButton = Button;
}

FKey UReignForceActivatableWidget::GetGamepadPromptKey(const UButton* Button, bool bFocused) const
{
    // Prompts of a menu covered by a submenu or a modal would show the keys which act on the menu over it
    if (!(IsActivated() && IsButtonAvailable(Button))) return EKeys::Invalid;
    if (bFocused) return EKeys::Gamepad_FaceButton_Bottom;
    if (Button == BackActionButton.Get()) return EKeys::Gamepad_FaceButton_Right;
    return EKeys::Invalid;
}

FKey UReignForceActivatableWidget::GetShownGamepadPrompt(const UButton* Button) const
{
    const FFocusableButtonState* State = FocusableButtons.FindByPredicate([&] (const FFocusableButtonState& Item) { return Item.Button == Button; });
    return State ? State->Prompt.GetShownKey() : EKeys::Invalid;
}

bool UReignForceActivatableWidget::IsGamepadUsed() const
{
    return UInputDeviceUtils::IsGamepadUsed(GetOwningLocalPlayer());
}

bool UReignForceActivatableWidget::IsButtonAvailable(const UButton* Button) const
{
    return IsValid(Button) && Button->GetIsEnabled() && Button->IsVisible();
}

void UReignForceActivatableWidget::FocusButton(UButton* Button)
{
    if (!IsButtonAvailable(Button)) return;
    LastFocusedButton = Button;
    Button->SetFocus();
}

void UReignForceActivatableWidget::RequestClose()
{
    OnCloseRequested.Broadcast();
}

void UReignForceActivatableWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    UpdateButtonsHighlight(InDeltaTime);
    UpdateScreenHeightLimit(MyGeometry);
    RestoreLostFocus();
}

void UReignForceActivatableWidget::UpdateScreenHeightLimit(const FGeometry& MyGeometry)
{
    if (!IsValid(ScreenHeightLimitedBox)) return;
    // The menu is fullscreen: its height is the screen one in the layout units
    const float Available = FMath::Max(MIN_SCREEN_LIMITED_HEIGHT, MyGeometry.GetLocalSize().Y - ScreenHeightMargin * 2);
    if (!FMath::IsNearlyEqual(ScreenHeightLimitedBox->GetMaxDesiredHeight(), Available, 1.f))
        ScreenHeightLimitedBox->SetMaxDesiredHeight(Available);
}

void UReignForceActivatableWidget::RestoreLostFocus()
{
    // Clicking an empty space or switching the input mode drops the focus, so keyboard & gamepad navigation would stop working
    const bool bFocusLost = bRestoreLostFocus && IsActivated() && !HasAnyUserFocus() && !HasFocusedDescendants();
    if (!bFocusLost) return;
    UWidget* Target = GetDesiredFocusTarget();
    if (IsValid(Target)) Target->SetFocus();
}

int32 UReignForceActivatableWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    int32 MaxLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
    // Children are painted already, so the focused button geometry is up to date
    for (const FFocusableButtonState& State : FocusableButtons)
    {
        if (!State.bHighlighted) continue;
        MaxLayer = FMath::Max(MaxLayer, FocusOutline::Paint(State.Button.Get(), FocusOutlineStyle, OutDrawElements, MaxLayer));
    }
    return MaxLayer;
}

void UReignForceActivatableWidget::NativeDestruct()
{
    CloseChildMenu();
    OnCloseRequested.Clear();

    Super::NativeDestruct();
}

UWidget* UReignForceActivatableWidget::NativeGetDesiredFocusTarget() const
{
    if (IsButtonAvailable(LastFocusedButton.Get())) return LastFocusedButton.Get();
    const FFocusableButtonState* Available = FocusableButtons.FindByPredicate([this] (const FFocusableButtonState& State)
    {
        return IsButtonAvailable(State.Button.Get());
    });
    return Available ? Available->Button.Get() : Super::NativeGetDesiredFocusTarget();
}

bool UReignForceActivatableWidget::NativeOnHandleBackAction()
{
    return HandleBackAction();
}

bool UReignForceActivatableWidget::HandleBackAction()
{
    if (!OnCloseRequested.IsBound()) return false;
    RequestClose();
    return true;
}

void UReignForceActivatableWidget::LimitHeightToScreen(USizeBox* Box, float ScreenMargin)
{
    ScreenHeightLimitedBox = Box;
    ScreenHeightMargin = ScreenMargin;
}

void UReignForceActivatableWidget::MakeMenuItemsScrollableCentered(UWidget* Item, float ScreenMargin)
{
    UPanelWidget* Panel = IsValid(Item) ? Item->GetParent() : nullptr;
    auto PanelSlot = IsValid(Panel) ? Cast<UCanvasPanelSlot>(Panel->Slot) : nullptr;
    if (!IsValid(PanelSlot)) return;

    const FAnchors Anchors = PanelSlot->GetAnchors();
    const FMargin Offsets = PanelSlot->GetOffsets();
    const bool bStretchedX = !FMath::IsNearlyEqual(Anchors.Minimum.X, Anchors.Maximum.X);

    USizeBox* Limit = MenuLayoutUtils::WrapInScrollBox(WidgetTree, Panel);
    auto LimitSlot = IsValid(Limit) ? Cast<UCanvasPanelSlot>(Limit->Slot) : nullptr;
    if (!IsValid(LimitSlot)) return;

    // The column takes its items height (up to the screen one) & stays in the screen middle; the width stays as designed
    if (!bStretchedX) Limit->SetWidthOverride(Offsets.Right + MenuLayoutUtils::SCROLLED_CONTENT_INSET * 2);
    LimitSlot->SetAnchors(FAnchors(Anchors.Minimum.X, .5f, Anchors.Maximum.X, .5f));
    LimitSlot->SetAlignment(FVector2D(LimitSlot->GetAlignment().X, .5f));
    LimitSlot->SetAutoSize(true);
    LimitSlot->SetOffsets(FMargin(Offsets.Left, 0, bStretchedX ? Offsets.Right : 0, 0));
    LimitHeightToScreen(Limit, ScreenMargin);
}

void UReignForceActivatableWidget::NativeOnActivated()
{
    Super::NativeOnActivated();
    if (bNavigationBlocked)
    {
        SetVisibility(VisibilityBeforeDeactivation);
        bNavigationBlocked = false;
    }
    // A button under the resting cursor is hovered as soon as the menu shows up, it shouldn't take the focus
    if (FSlateApplication::IsInitialized()) HoverCursorPosition = FSlateApplication::Get().GetCursorPos();
}

void UReignForceActivatableWidget::NativeOnDeactivated()
{
    Super::NativeOnDeactivated();
    // A menu covered by a submenu or modal is still on screen: its buttons would be found by the arrows & gamepad
    // navigation of the menu over it (it searches by position) and the focus would bounce back
    const ESlateVisibility Current = GetVisibility();
    const bool bCanBlock = !bNavigationBlocked && Current != ESlateVisibility::Collapsed && Current != ESlateVisibility::Hidden;
    if (!bCanBlock) return;
    VisibilityBeforeDeactivation = Current;
    bNavigationBlocked = true;
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

bool UReignForceActivatableWidget::IsHoverFromMouseMove()
{
    // Gamepad virtual cursor hovers the buttons too, only the real mouse moves the focus
    if (!FSlateApplication::IsInitialized() || IsGamepadUsed()) return false;

    const FVector2D CursorPosition = FSlateApplication::Get().GetCursorPos();
    if (CursorPosition.Equals(HoverCursorPosition, 1.)) return false;
    HoverCursorPosition = CursorPosition;
    return true;
}

void UReignForceActivatableWidget::HandleButtonHovered()
{
    if (!IsActivated() || !IsHoverFromMouseMove()) return;
    const FFocusableButtonState* Hovered = FocusableButtons.FindByPredicate([this] (const FFocusableButtonState& State)
    {
        const UButton* Button = State.Button.Get();
        return IsButtonAvailable(Button) && Button->IsHovered() && !Button->HasAnyUserFocus();
    });
    if (Hovered) FocusButton(Hovered->Button.Get());
}

void UReignForceActivatableWidget::UpdateButtonsHighlight(float DeltaTime)
{
    const bool bGamepad = IsGamepadUsed();
    for (FFocusableButtonState& State : FocusableButtons)
    {
        UButton* Button = State.Button.Get();
        if (!IsValid(Button)) continue;
        const bool bFocused = Button->HasAnyUserFocus() && Button->GetIsEnabled();

        // Gamepad prompts mark what the buttons do: "(A) Apply" on the focused one, "(B) Back" on the back action one
        State.Prompt.Update(Button, bGamepad ? GetGamepadPromptKey(Button, bFocused) : EKeys::Invalid, DeltaTime);
        if (bFocused == State.bHighlighted) continue;

        State.bHighlighted = bFocused;
        if (!bFocused) continue;
        LastFocusedButton = Button;
        // Hover sound is played by the button itself, keyboard & gamepad navigation needs it too
        if (!Button->IsHovered() && FSlateApplication::IsInitialized())
            FSlateApplication::Get().PlaySound(State.DefaultStyle.HoveredSlateSound);
    }
}

void UReignForceActivatableWidget::PushChildMenu(UReignForceActivatableWidget* Child, int32 ZOrder)
{
    if (!IsValid(Child)) return;
    CloseChildMenu();
    // The button which opened the submenu gets the focus back when it is closed
    ChildMenuOpener = LastFocusedButton;
    ChildMenu = Child;
    ChildMenu->OnCloseRequested.AddDynamic(this, &UReignForceActivatableWidget::CloseChildMenu);
    ChildMenu->AddToViewport(ZOrder);
    DeactivateWidget();
}

void UReignForceActivatableWidget::CloseChildMenu()
{
    if (!IsValid(ChildMenu)) return;
    ChildMenu->OnCloseRequested.RemoveDynamic(this, &UReignForceActivatableWidget::CloseChildMenu);
    ChildMenu->RemoveFromParent();
    ChildMenu = nullptr;
    if (IsInViewport() && !IsActivated()) ActivateWidget();
    FocusButton(ChildMenuOpener.Get());
    ChildMenuOpener = nullptr;
}

void UReignForceActivatableWidget::OpenSettingsMenu(UButton* StyleTemplate)
{
    if (!IsValid(SettingsMenuClass)) return;
    auto Settings = CreateWidget<UGameSettingsWidget>(GetOwningPlayer(), SettingsMenuClass);
    if (!IsValid(Settings)) return;
    Settings->Setup(StyleTemplate);
    PushChildMenu(Settings);
}
