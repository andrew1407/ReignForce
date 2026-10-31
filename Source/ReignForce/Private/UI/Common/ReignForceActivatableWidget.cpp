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
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "CommonTextBlock.h"

#include "UI/Settings/GameSettingsWidget.h"


namespace
{
    FSlateColor TintSlateColor(const FSlateColor& Color, const FLinearColor& Tint)
    {
        return Color.IsColorSpecified() ? FSlateColor(Color.GetSpecifiedColor() * Tint) : Color;
    }
}

UReignForceActivatableWidget::UReignForceActivatableWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bAutoActivate = true;
    bIsBackHandler = true;
    bRestoreLostFocus = true;

    FocusedButtonScale = 1.12f;
    FocusedButtonTint = FLinearColor(1.35f, 1.25f, 1.f, 1.f);
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
    State.DefaultScale = Button->GetRenderTransform().Scale;

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

    // Clicking an empty space or switching the input mode drops the focus, so keyboard & gamepad navigation would stop working
    bool bFocusLost = bRestoreLostFocus && IsActivated() && !HasAnyUserFocus() && !HasFocusedDescendants();
    if (!bFocusLost) return;
    UWidget* Target = GetDesiredFocusTarget();
    if (IsValid(Target)) Target->SetFocus();
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
    Button->SetToolTipText(Label);

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
    Parent->InsertChildAt(Index, Button, Template->Slot);
    return Button;
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

void UReignForceActivatableWidget::HandleButtonHovered()
{
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
        SetButtonHighlight(State, bFocused);
    }
}

void UReignForceActivatableWidget::SetButtonHighlight(FFocusableButtonState& State, bool bHighlight)
{
    UButton* Button = State.Button.Get();
    State.bHighlighted = bHighlight;
    if (!IsValid(Button)) return;

    if (!bHighlight)
    {
        Button->SetStyle(State.DefaultStyle);
        Button->SetRenderScale(State.DefaultScale);
        return;
    }

    FButtonStyle Style = State.DefaultStyle;
    Style.Normal = State.DefaultStyle.Hovered;
    Style.Normal.TintColor = TintSlateColor(State.DefaultStyle.Hovered.TintColor, FocusedButtonTint);
    Style.Hovered.TintColor = Style.Normal.TintColor;
    Button->SetStyle(Style);
    Button->SetRenderTransformPivot(FVector2D(.5f));
    Button->SetRenderScale(State.DefaultScale * FocusedButtonScale);
}

bool UReignForceActivatableWidget::IsButtonAvailable(const UButton* Button) const
{
    return IsValid(Button) && Button->GetIsEnabled() && Button->IsVisible();
}

void UReignForceActivatableWidget::PushChildMenu(UReignForceActivatableWidget* Child, int32 ZOrder)
{
    if (!IsValid(Child)) return;
    CloseChildMenu();
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
}

void UReignForceActivatableWidget::OpenSettingsMenu(UButton* StyleTemplate)
{
    if (!IsValid(SettingsMenuClass)) return;
    auto Settings = CreateWidget<UGameSettingsWidget>(GetOwningPlayer(), SettingsMenuClass);
    if (!IsValid(Settings)) return;
    Settings->Setup(StyleTemplate);
    PushChildMenu(Settings);
}
