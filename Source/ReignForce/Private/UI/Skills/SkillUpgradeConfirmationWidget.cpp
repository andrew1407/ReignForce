// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skills/SkillUpgradeConfirmationWidget.h"
#include "Blueprint/WidgetTree.h"

#include "Components/BackgroundBlur.h"
#include "Components/ButtonSlot.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"


USkillUpgradeConfirmationWidget::USkillUpgradeConfirmationWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bIsModal = true;
    bBuilt = false;
    BlurStrength = 6;
    PanelWidth = 560;

    // Pause menu confirmation modals palette (warm tint, dark red framed orange box, dark red rounded buttons)
    DimColor = FLinearColor(.495f, .27f, .075f, .4f);
    FrameColor = FLinearColor(.052f, 0, .006f, 1);
    PanelColor = FLinearColor(.495f, .2f, 0, 1);
    ButtonColor = FLinearColor(.229f, .059f, .036f, 1);
    ButtonHoveredColor = FLinearColor(.724f, .724f, .724f, 1);
    ButtonPressedColor = FLinearColor(.384f, .384f, .384f, 1);
    ButtonOutlineColor = FLinearColor(.695f, .695f, .695f, 1);
}

FButtonStyle USkillUpgradeConfirmationWidget::MakeModalButtonStyle() const
{
    auto MakeBrush = [](const FLinearColor& Color, const FLinearColor& Outline)
    {
        FSlateBrush Brush;
        Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
        Brush.TintColor = Color;
        Brush.OutlineSettings = FSlateBrushOutlineSettings(FVector4(4, 4, 4, 4), FSlateColor(Outline), 1);
        Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
        return Brush;
    };

    FButtonStyle Style;
    Style.SetNormal(MakeBrush(ButtonColor, ButtonOutlineColor));
    Style.SetHovered(MakeBrush(ButtonHoveredColor, ButtonHoveredColor));
    Style.SetPressed(MakeBrush(ButtonPressedColor, ButtonHoveredColor));
    Style.SetDisabled(MakeBrush(ButtonColor * FLinearColor(1, 1, 1, .5f), ButtonOutlineColor));
    Style.SetNormalPadding(FMargin(12, 1.5f));
    Style.SetPressedPadding(FMargin(12, 1.5f));
    return Style;
}

UButton* USkillUpgradeConfirmationWidget::MakeModalButton(const FText& Label, const FSlateFontInfo& Font, FName Name)
{
    UTextBlock* Text = MakeText(Label, Font, FLinearColor::White);
    UButton* Button = MakeButton(nullptr, Text, Name);
    Button->SetStyle(MakeModalButtonStyle());
    if (auto TextSlot = Cast<UButtonSlot>(Text->Slot))
    {
        TextSlot->SetPadding(FMargin(4, 6));
        TextSlot->SetHorizontalAlignment(HAlign_Center);
        TextSlot->SetVerticalAlignment(VAlign_Center);
    }
    return Button;
}

TSharedRef<SWidget> USkillUpgradeConfirmationWidget::RebuildWidget()
{
    if (!bBuilt) Setup(nullptr, FText::FromString(TEXT("Upgrade the skill?")), FText::GetEmpty());
    return Super::RebuildWidget();
}

void USkillUpgradeConfirmationWidget::NativeDestruct()
{
    OnConfirmation.Clear();
    Super::NativeDestruct();
}

bool USkillUpgradeConfirmationWidget::HandleBackAction()
{
    HandleCancel();
    return true;
}

void USkillUpgradeConfirmationWidget::Setup(UButton* StyleTemplate, const FText& SkillTitle, const FText& Details)
{
    // Roboto Bold like the pause menu modals: the template (skill upgrade button) look is not reused, its yellow style mismatches the dialogs
    (void)StyleTemplate;
    if (bBuilt || !IsValid(WidgetTree)) return;
    bBuilt = true;

    const FSlateFontInfo TitleFont = MakeFont(nullptr, 24);
    const FSlateFontInfo DetailsFont = MakeFont(nullptr, 18);
    const FSlateFontInfo ButtonFont = MakeFont(nullptr, 18);
    const FSlateColor TextColor = FLinearColor::White;

    auto Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
    WidgetTree->RootWidget = Root;

    // The skills menu underneath is blurred & tinted like the pause menu modals background, the dialog stays sharp over it
    auto Blur = WidgetTree->ConstructWidget<UBackgroundBlur>(UBackgroundBlur::StaticClass(), TEXT("Blur"));
    Blur->SetBlurStrength(BlurStrength);
    auto Dim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
    Dim->SetBrushColor(DimColor);
    for (UWidget* Layer : { static_cast<UWidget*>(Blur), static_cast<UWidget*>(Dim) })
    {
        if (UOverlaySlot* LayerSlot = Root->AddChildToOverlay(Layer))
        {
            LayerSlot->SetHorizontalAlignment(HAlign_Fill);
            LayerSlot->SetVerticalAlignment(VAlign_Fill);
        }
    }

    auto Frame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Frame"));
    Frame->SetWidthOverride(PanelWidth);
    if (UOverlaySlot* FrameSlot = Root->AddChildToOverlay(Frame))
    {
        FrameSlot->SetHorizontalAlignment(HAlign_Center);
        FrameSlot->SetVerticalAlignment(VAlign_Center);
    }

    // Dark red border around the orange message box
    auto MessageBoxBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MessageBoxBorder"));
    MessageBoxBorder->SetBrushColor(FrameColor);
    MessageBoxBorder->SetPadding(FMargin(4));
    Frame->SetContent(MessageBoxBorder);

    auto Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MessageBox"));
    Panel->SetBrushColor(PanelColor);
    Panel->SetPadding(FMargin(32, 26));
    MessageBoxBorder->SetContent(Panel);

    auto Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
    Panel->SetContent(Column);

    UTextBlock* Title = MakeText(SkillTitle, TitleFont, TextColor);
    Title->SetJustification(ETextJustify::Center);
    Title->SetAutoWrapText(true);
    if (UVerticalBoxSlot* TitleSlot = Column->AddChildToVerticalBox(Title))
        TitleSlot->SetHorizontalAlignment(HAlign_Center);

    if (!Details.IsEmpty())
    {
        UTextBlock* DetailsText = MakeText(Details, DetailsFont, FLinearColor(1, 1, 1, .9f));
        DetailsText->SetJustification(ETextJustify::Center);
        DetailsText->SetAutoWrapText(true);
        if (UVerticalBoxSlot* DetailsSlot = Column->AddChildToVerticalBox(DetailsText))
        {
            DetailsSlot->SetHorizontalAlignment(HAlign_Center);
            DetailsSlot->SetPadding(FMargin(0, 12, 0, 0));
        }
    }

    auto Actions = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Actions"));
    AcceptButton = MakeModalButton(FText::FromString(TEXT("Upgrade")), ButtonFont, TEXT("AcceptButton"));
    CancelButton = MakeModalButton(FText::FromString(TEXT("Cancel")), ButtonFont, TEXT("CancelButton"));
    AcceptButton->OnClicked.AddDynamic(this, &USkillUpgradeConfirmationWidget::HandleAccept);
    CancelButton->OnClicked.AddDynamic(this, &USkillUpgradeConfirmationWidget::HandleCancel);
    for (UButton* Button : { AcceptButton.Get(), CancelButton.Get() })
    {
        if (UHorizontalBoxSlot* ActionSlot = Actions->AddChildToHorizontalBox(Button))
        {
            ActionSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            ActionSlot->SetPadding(FMargin(24, 0));
        }
    }
    if (UVerticalBoxSlot* ActionsSlot = Column->AddChildToVerticalBox(Actions))
        ActionsSlot->SetPadding(FMargin(0, 26, 0, 10));

    // The upgrade was asked for explicitly, so it gets the initial focus (A twice upgrades)
    RegisterFocusableButton(AcceptButton);
    RegisterFocusableButton(CancelButton);
    FocusButton(AcceptButton);
}

void USkillUpgradeConfirmationWidget::HandleAccept()
{
    constexpr bool bAccepted = true;
    OnConfirmation.Broadcast(bAccepted);
}

void USkillUpgradeConfirmationWidget::HandleCancel()
{
    constexpr bool bAccepted = false;
    OnConfirmation.Broadcast(bAccepted);
}
