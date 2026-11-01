// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/ReignForceActivatableWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "Blueprint/WidgetTree.h"
#include "Styling/CoreStyle.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/PanelWidget.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/ContentWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "CommonTextBlock.h"
#include "CommonInputSubsystem.h"

#include "UI/Settings/GameSettingsWidget.h"


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
    bool bRegistered = FocusableButtons.ContainsByPredicate([&] (const FFocusableButtonState& State)
    {
        return State.Button == Button;
    });
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

    Button->OnHovered.AddDynamic(this, &UReignForceActivatableWidget::HandleButtonHovered);
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

    UpdateButtonsHighlight();

    // The menu is fullscreen: its height is the screen one in the layout units
    if (IsValid(ScreenHeightLimitedBox))
    {
        constexpr float MinHeight = 120;
        const float Available = FMath::Max(MinHeight, MyGeometry.GetLocalSize().Y - ScreenHeightMargin * 2);
        if (!FMath::IsNearlyEqual(ScreenHeightLimitedBox->GetMaxDesiredHeight(), Available, 1.f))
            ScreenHeightLimitedBox->SetMaxDesiredHeight(Available);
    }

    // Clicking an empty space or switching the input mode drops the focus, so keyboard & gamepad navigation would stop working
    bool bFocusLost = bRestoreLostFocus && IsActivated() && !HasAnyUserFocus() && !HasFocusedDescendants();
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
        const int32 OutlineLayer = FocusOutline::Paint(State.Button.Get(), FocusOutlineStyle, OutDrawElements, MaxLayer);
        MaxLayer = FMath::Max(MaxLayer, OutlineLayer);
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
    for (const FFocusableButtonState& State : FocusableButtons)
    {
        if (IsButtonAvailable(State.Button.Get())) return State.Button.Get();
    }
    return Super::NativeGetDesiredFocusTarget();
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

UButton* UReignForceActivatableWidget::CreateButtonLike(UButton* Template, const FText& Label, FName Name, int32 IndexOffset)
{
    if (!(IsValid(Template) && IsValid(WidgetTree))) return nullptr;
    UPanelWidget* Parent = Template->GetParent();
    if (!IsValid(Parent)) return nullptr;

    auto Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
    Button->SetStyle(Template->GetStyle());
    Button->SetColorAndOpacity(Template->GetColorAndOpacity());
    Button->SetBackgroundColor(Template->GetBackgroundColor());

    auto TemplateLabel = Cast<UTextBlock>(Template->GetChildAt(0));
    auto Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    Text->SetText(Label);
    if (IsValid(TemplateLabel))
    {
        Text->SetFont(TemplateLabel->GetFont());
        Text->SetColorAndOpacity(TemplateLabel->GetColorAndOpacity());
        Text->SetShadowOffset(TemplateLabel->GetShadowOffset());
        Text->SetShadowColorAndOpacity(TemplateLabel->GetShadowColorAndOpacity());
        Button->AddChild(Text, TemplateLabel->Slot);
    }
    else
    {
        Button->AddChild(Text);
    }

    const int32 Index = FMath::Clamp(Parent->GetChildIndex(Template) + IndexOffset, 0, Parent->GetChildrenCount());
    // Menu items are separated by spacers, so the new button gets its own one on the side the neighbour spacer is
    UWidget* Previous = Index > 0 ? Parent->GetChildAt(Index - 1) : nullptr;
    UWidget* Next = Index < Parent->GetChildrenCount() ? Parent->GetChildAt(Index) : nullptr;
    auto PreviousSpacer = Cast<USpacer>(Previous);
    auto NextSpacer = Cast<USpacer>(Next);
    Parent->InsertChildAt(Index, Button, Template->Slot);

    USpacer* SpacerTemplate = IsValid(PreviousSpacer) ? PreviousSpacer : NextSpacer;
    if (!IsValid(SpacerTemplate)) return Button;
    auto Spacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass());
    Spacer->SetSize(SpacerTemplate->GetSize());
    // [spacer, new, template] -> [spacer, new, spacer, template]; [template, new, spacer] -> [template, spacer, new, spacer]
    const int32 SpacerIndex = IsValid(PreviousSpacer) ? Index + 1 : Index;
    Parent->InsertChildAt(SpacerIndex, Spacer, SpacerTemplate->Slot);
    return Button;
}

void UReignForceActivatableWidget::SetMenuItemsGap(UWidget* Item, float Gap)
{
    UPanelWidget* Panel = IsValid(Item) ? Item->GetParent() : nullptr;
    if (!IsValid(Panel)) return;

    // Items spread over the whole column height (fill) would ignore the gaps, so each takes only its own height
    const TArray<UWidget*> Children = Panel->GetAllChildren();
    bool bHasSpacers = Children.ContainsByPredicate([] (const UWidget* Child) { return Child->IsA<USpacer>(); });
    for (UWidget* Child : Children)
    {
        auto ItemSlot = Cast<UVerticalBoxSlot>(Child->Slot);
        if (!IsValid(ItemSlot)) continue;
        ItemSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
        FMargin ItemPadding = ItemSlot->GetPadding();
        if (auto Spacer = Cast<USpacer>(Child))
        {
            Spacer->SetSize(FVector2D(Spacer->GetSize().X, Gap));
            ItemPadding.Top = ItemPadding.Bottom = 0;
        }
        else
        {
            // Without spacers the gap is made of the items paddings
            ItemPadding.Top = ItemPadding.Bottom = bHasSpacers ? 0 : Gap * .5f;
        }
        ItemSlot->SetPadding(ItemPadding);
    }
}

USizeBox* UReignForceActivatableWidget::WrapInScrollBox(UWidget* Content, UScrollBox** OutScrollBox)
{
    if (!(IsValid(Content) && IsValid(WidgetTree))) return nullptr;

    auto Limit = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
    auto Scroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass());
    // Keyboard & gamepad navigation reaches the items out of the view and scrolls them in
    Scroll->SetScrollWhenFocusChanges(EScrollWhenFocusChanges::AnimatedScroll);
    Scroll->SetNavigationDestination(EDescendantScrollDestination::IntoView);
    Scroll->SetAnimateWheelScrolling(true);

    // The size box takes the content place with the same slot settings (layout on the parent stays the same)
    if (UPanelWidget* Parent = Content->GetParent())
    {
        if (auto SingleChildParent = Cast<UContentWidget>(Parent))
        {
            SingleChildParent->SetContent(Limit);
        }
        else
        {
            Parent->InsertChildAt(Parent->GetChildIndex(Content), Limit, Content->Slot);
            Parent->RemoveChild(Content);
        }
    }
    else if (WidgetTree->RootWidget == Content)
    {
        WidgetTree->RootWidget = Limit;
    }
    Limit->SetContent(Scroll);
    // Focus frames are drawn outside the items & clipped by the scroll box, the inset keeps the edge ones whole
    if (auto ContentSlot = Cast<UScrollBoxSlot>(Scroll->AddChild(Content)))
        ContentSlot->SetPadding(FMargin(SCROLLED_CONTENT_INSET));

    if (OutScrollBox) *OutScrollBox = Scroll;
    return Limit;
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
    bool bStretchedX = !FMath::IsNearlyEqual(Anchors.Minimum.X, Anchors.Maximum.X);

    USizeBox* Limit = WrapInScrollBox(Panel);
    auto LimitSlot = IsValid(Limit) ? Cast<UCanvasPanelSlot>(Limit->Slot) : nullptr;
    if (!IsValid(LimitSlot)) return;

    // The column takes its items height (up to the screen one) & stays in the screen middle; the width stays as designed
    if (!bStretchedX) Limit->SetWidthOverride(Offsets.Right + SCROLLED_CONTENT_INSET * 2);
    LimitSlot->SetAnchors(FAnchors(Anchors.Minimum.X, .5f, Anchors.Maximum.X, .5f));
    LimitSlot->SetAlignment(FVector2D(LimitSlot->GetAlignment().X, .5f));
    LimitSlot->SetAutoSize(true);
    LimitSlot->SetOffsets(FMargin(Offsets.Left, 0, bStretchedX ? Offsets.Right : 0, 0));
    LimitHeightToScreen(Limit, ScreenMargin);
}

FSlateFontInfo UReignForceActivatableWidget::MakeFont(const UButton* StyleTemplate, int32 Size)
{
    auto TemplateLabel = IsValid(StyleTemplate) ? Cast<UTextBlock>(StyleTemplate->GetChildAt(0)) : nullptr;
    FSlateFontInfo Font = IsValid(TemplateLabel) ? TemplateLabel->GetFont() : FCoreStyle::GetDefaultFontStyle("Bold", Size);
    Font.Size = Size;
    return Font;
}

UButton* UReignForceActivatableWidget::MakeButton(UButton* StyleTemplate, UWidget* Content, FName Name)
{
    auto Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
    if (IsValid(StyleTemplate))
    {
        Button->SetStyle(StyleTemplate->GetStyle());
        Button->SetColorAndOpacity(StyleTemplate->GetColorAndOpacity());
        Button->SetBackgroundColor(StyleTemplate->GetBackgroundColor());
    }
    if (UButtonSlot* ContentSlot = Cast<UButtonSlot>(Button->AddChild(Content)))
    {
        ContentSlot->SetPadding(FMargin(18, 6));
        ContentSlot->SetHorizontalAlignment(Content->IsA<UTextBlock>() ? HAlign_Center : HAlign_Fill);
        ContentSlot->SetVerticalAlignment(VAlign_Center);
    }
    return Button;
}

UTextBlock* UReignForceActivatableWidget::MakeText(const FText& Text, const FSlateFontInfo& Font, const FSlateColor& Color)
{
    auto TextBlock = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass());
    TextBlock->SetText(Text);
    TextBlock->SetFont(Font);
    TextBlock->SetColorAndOpacity(Color);
    TextBlock->SetShadowOffset(FVector2D(1));
    TextBlock->SetShadowColorAndOpacity(FLinearColor(0, 0, 0, .6f));
    return TextBlock;
}

UVerticalBox* UReignForceActivatableWidget::ConstructMenuColumn(const FText& Title, const FSlateFontInfo& TitleFont, float Width, const FLinearColor& Background)
{
    auto BackgroundBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
    BackgroundBorder->SetBrushColor(Background);
    BackgroundBorder->SetHorizontalAlignment(HAlign_Center);
    BackgroundBorder->SetVerticalAlignment(VAlign_Center);
    WidgetTree->RootWidget = BackgroundBorder;

    auto Frame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Frame"));
    Frame->SetWidthOverride(Width);
    BackgroundBorder->SetContent(Frame);

    auto Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
    Frame->SetContent(Column);

    UTextBlock* TitleText = MakeText(Title, TitleFont, FLinearColor::White);
    if (UVerticalBoxSlot* TitleSlot = Column->AddChildToVerticalBox(TitleText))
    {
        TitleSlot->SetHorizontalAlignment(HAlign_Center);
        TitleSlot->SetPadding(FMargin(0, 0, 0, 24));
    }
    return Column;
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
    bool bCanBlock = !bNavigationBlocked && Current != ESlateVisibility::Collapsed && Current != ESlateVisibility::Hidden;
    if (!bCanBlock) return;
    VisibilityBeforeDeactivation = Current;
    bNavigationBlocked = true;
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

bool UReignForceActivatableWidget::IsHoverFromMouseMove()
{
    if (!FSlateApplication::IsInitialized()) return false;
    // Gamepad virtual cursor hovers the buttons too, only the real mouse moves the focus
    auto InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
    if (InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad) return false;

    const FVector2D CursorPosition = FSlateApplication::Get().GetCursorPos();
    if (CursorPosition.Equals(HoverCursorPosition, 1.)) return false;
    HoverCursorPosition = CursorPosition;
    return true;
}

void UReignForceActivatableWidget::HandleButtonHovered()
{
    if (!IsActivated() || !IsHoverFromMouseMove()) return;
    for (const FFocusableButtonState& State : FocusableButtons)
    {
        UButton* Button = State.Button.Get();
        bool bHoveredByMouse = IsButtonAvailable(Button) && Button->IsHovered();
        if (bHoveredByMouse && !Button->HasAnyUserFocus()) return FocusButton(Button);
    }
}

void UReignForceActivatableWidget::UpdateButtonsHighlight()
{
    for (FFocusableButtonState& State : FocusableButtons)
    {
        UButton* Button = State.Button.Get();
        if (!IsValid(Button)) continue;
        bool bFocused = Button->HasAnyUserFocus() && Button->GetIsEnabled();
        if (bFocused == State.bHighlighted) continue;

        if (bFocused)
        {
            LastFocusedButton = Button;
            // Hover sound is played by the button itself, keyboard & gamepad navigation needs it too
            if (!Button->IsHovered() && FSlateApplication::IsInitialized())
                FSlateApplication::Get().PlaySound(State.DefaultStyle.HoveredSlateSound);
        }
        State.bHighlighted = bFocused;
    }
}

bool UReignForceActivatableWidget::IsButtonAvailable(const UButton* Button) const
{
    return IsValid(Button) && Button->GetIsEnabled() && Button->IsVisible();
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
    if (IsButtonAvailable(ChildMenuOpener.Get())) FocusButton(ChildMenuOpener.Get());
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
