// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Rounds/StartRoundWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "CommonInputSubsystem.h"

#include "UI/Common/ReignForceGamepadControllerData.h"


namespace
{
    constexpr float ICON_GAP = 10;
}

UStartRoundWidget::UStartRoundWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    StartActionKey = TEXT("T");
    StartActionGamepadKey = EKeys::Gamepad_DPad_Down;
}

void UStartRoundWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BuildGamepadPrompt();
    bGamepadPrompt = false;
    SetGamepadPrompt(IsGamepadUsed());
}

void UStartRoundWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    const bool bGamepad = IsGamepadUsed();
    if (bGamepad != bGamepadPrompt) SetGamepadPrompt(bGamepad);
}

FText UStartRoundWidget::GetDescriptionMessage() const
{
    return FText::FromString(FString::Printf(TEXT("Press %s to start a new round."), *StartActionKey.ToString()));
}

void UStartRoundWidget::UpdateRoundTextBlockDescrition()
{
    if (!IsValid(RoundStartMessage)) return;
    RoundStartMessage->SetText(bGamepadPrompt && IsValid(GamepadIcon) ? FText::FromString(TEXT("Press")) : GetDescriptionMessage());
}

void UStartRoundWidget::BuildGamepadPrompt()
{
    if (IsValid(GamepadIcon) || !IsValid(RoundStartMessage) || !IsValid(WidgetTree)) return;
    UPanelWidget* Parent = RoundStartMessage->GetParent();
    if (!IsValid(Parent)) return;

    FSlateBrush Brush;
    if (!UReignForceGamepadControllerData::GetGamepadKeyBrush(StartActionGamepadKey, Brush)) return;
    // The icon is about the text line height
    const float IconSize = FMath::Max(16.f, RoundStartMessage->GetFont().Size * 1.6f);
    Brush.ImageSize = FVector2D(IconSize);

    // The row takes the message place (its slot & layout in the blueprint), the message moves into the row
    auto Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("RoundStartPrompt"));
    // (ReplaceChildAt is editor only: the row is inserted with the message slot settings, then the message leaves the parent)
    Parent->InsertChildAt(Parent->GetChildIndex(RoundStartMessage), Row, RoundStartMessage->Slot);
    Parent->RemoveChild(RoundStartMessage);

    GamepadIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("RoundStartGamepadIcon"));
    GamepadIcon->SetBrush(Brush);
    GamepadSuffix = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RoundStartGamepadSuffix"));
    GamepadSuffix->SetText(FText::FromString(TEXT("to start a new round.")));
    GamepadSuffix->SetFont(RoundStartMessage->GetFont());
    GamepadSuffix->SetColorAndOpacity(RoundStartMessage->GetColorAndOpacity());
    GamepadSuffix->SetShadowOffset(RoundStartMessage->GetShadowOffset());
    GamepadSuffix->SetShadowColorAndOpacity(RoundStartMessage->GetShadowColorAndOpacity());

    for (UWidget* Part : { static_cast<UWidget*>(RoundStartMessage.Get()), static_cast<UWidget*>(GamepadIcon.Get()), static_cast<UWidget*>(GamepadSuffix.Get()) })
    {
        if (UHorizontalBoxSlot* PartSlot = Row->AddChildToHorizontalBox(Part))
        {
            PartSlot->SetVerticalAlignment(VAlign_Center);
            if (Part == GamepadIcon) PartSlot->SetPadding(FMargin(ICON_GAP, 0));
        }
    }
}

void UStartRoundWidget::SetGamepadPrompt(bool bGamepad)
{
    bGamepadPrompt = bGamepad && IsValid(GamepadIcon);
    const ESlateVisibility GamepadParts = bGamepadPrompt ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
    if (IsValid(GamepadIcon)) GamepadIcon->SetVisibility(GamepadParts);
    if (IsValid(GamepadSuffix)) GamepadSuffix->SetVisibility(GamepadParts);
    UpdateRoundTextBlockDescrition();
}

bool UStartRoundWidget::IsGamepadUsed() const
{
    const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
    return InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;
}
