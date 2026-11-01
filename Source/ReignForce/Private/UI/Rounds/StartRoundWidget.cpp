// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Rounds/StartRoundWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "CommonInputSubsystem.h"

#include "UI/Common/ReignForceGamepadControllerData.h"


namespace
{
    constexpr float ICON_GAP = 10;
    /** Key text / gamepad icon cross-fade time (seconds) */
    constexpr float KEY_BLEND_TIME = .2f;
}

UStartRoundWidget::UStartRoundWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    StartActionKey = TEXT("T");
    StartActionGamepadKey = EKeys::Gamepad_LeftShoulder;
}

void UStartRoundWidget::NativeConstruct()
{
    Super::NativeConstruct();

    BuildGamepadPrompt();
    bGamepadPrompt = IsGamepadUsed() && IsValid(GamepadIcon);
    GamepadBlend = bGamepadPrompt ? 1.f : 0.f;
    UpdateRoundTextBlockDescrition();
    ApplyKeyBlend();
}

void UStartRoundWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    const bool bGamepad = IsGamepadUsed();
    if (bGamepad != bGamepadPrompt) SetGamepadPrompt(bGamepad);

    // The key slot cross-fades between the key name & the gamepad icon, the text around it stays still
    const float Target = bGamepadPrompt ? 1.f : 0.f;
    if (GamepadBlend == Target) return;
    GamepadBlend = FMath::FInterpConstantTo(GamepadBlend, Target, InDeltaTime, 1.f / KEY_BLEND_TIME);
    ApplyKeyBlend();
}

FText UStartRoundWidget::GetDescriptionMessage() const
{
    return FText::FromString(FString::Printf(TEXT("Press %s to start a new round."), *StartActionKey.ToString()));
}

void UStartRoundWidget::UpdateRoundTextBlockDescrition()
{
    if (!IsValid(RoundStartMessage)) return;
    // With the prompt row the message is only its first word, the key & the rest are the row parts
    RoundStartMessage->SetText(IsValid(KeyText) ? FText::FromString(TEXT("Press")) : GetDescriptionMessage());
    if (IsValid(KeyText)) KeyText->SetText(FText::FromName(StartActionKey));
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

    auto MakePart = [this] (const TCHAR* Name)
    {
        auto Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
        Text->SetFont(RoundStartMessage->GetFont());
        Text->SetColorAndOpacity(RoundStartMessage->GetColorAndOpacity());
        Text->SetShadowOffset(RoundStartMessage->GetShadowOffset());
        Text->SetShadowColorAndOpacity(RoundStartMessage->GetShadowColorAndOpacity());
        return Text;
    };
    KeyText = MakePart(TEXT("RoundStartKey"));
    GamepadSuffix = MakePart(TEXT("RoundStartSuffix"));
    GamepadSuffix->SetText(FText::FromString(TEXT("to start a new round.")));

    // Key slot as wide as the icon: the key name & the icon share it, so switching the device doesn't move the text
    auto KeySlotBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RoundStartKeySlot"));
    KeySlotBox->SetMinDesiredWidth(IconSize);
    KeySlotBox->SetMinDesiredHeight(IconSize);
    auto KeyOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RoundStartKeyOverlay"));
    KeySlotBox->SetContent(KeyOverlay);
    GamepadIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("RoundStartGamepadIcon"));
    GamepadIcon->SetBrush(Brush);
    for (UWidget* Key : { static_cast<UWidget*>(KeyText.Get()), static_cast<UWidget*>(GamepadIcon.Get()) })
    {
        if (UOverlaySlot* KeyPartSlot = KeyOverlay->AddChildToOverlay(Key))
        {
            KeyPartSlot->SetHorizontalAlignment(HAlign_Center);
            KeyPartSlot->SetVerticalAlignment(VAlign_Center);
        }
    }

    for (UWidget* Part : { static_cast<UWidget*>(RoundStartMessage.Get()), static_cast<UWidget*>(KeySlotBox), static_cast<UWidget*>(GamepadSuffix.Get()) })
    {
        if (UHorizontalBoxSlot* PartSlot = Row->AddChildToHorizontalBox(Part))
        {
            PartSlot->SetVerticalAlignment(VAlign_Center);
            if (Part == KeySlotBox) PartSlot->SetPadding(FMargin(ICON_GAP, 0));
        }
    }
}

void UStartRoundWidget::SetGamepadPrompt(bool bGamepad)
{
    bGamepadPrompt = bGamepad && IsValid(GamepadIcon);
    UpdateRoundTextBlockDescrition();
}

void UStartRoundWidget::ApplyKeyBlend()
{
    const float Blend = FMath::InterpEaseInOut(0.f, 1.f, GamepadBlend, 2.f);
    if (IsValid(KeyText)) KeyText->SetRenderOpacity(1 - Blend);
    if (IsValid(GamepadIcon)) GamepadIcon->SetRenderOpacity(Blend);
}

bool UStartRoundWidget::IsGamepadUsed() const
{
    const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
    return InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;
}
