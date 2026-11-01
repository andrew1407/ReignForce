// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skills/SkillUpgradeConfirmationWidget.h"
#include "Blueprint/WidgetTree.h"

#include "Components/BackgroundBlur.h"
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
    BlurStrength = 8;
    DimColor = FLinearColor(0, 0, 0, .35f);
    PanelColor = FLinearColor(0, 0, 0, .8f);
    PanelWidth = 560;
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
    if (bBuilt || !IsValid(WidgetTree)) return;
    bBuilt = true;

    const FSlateFontInfo TitleFont = MakeFont(StyleTemplate, 30);
    const FSlateFontInfo DetailsFont = MakeFont(StyleTemplate, 20);
    const FSlateFontInfo ButtonFont = MakeFont(StyleTemplate, 22);
    const FSlateFontInfo HintFont = MakeFont(StyleTemplate, 14);
    const FSlateColor TextColor = FLinearColor::White;

    auto Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
    WidgetTree->RootWidget = Root;

    // The skills menu underneath is blurred & dimmed, the dialog stays sharp over it
    auto Blur = WidgetTree->ConstructWidget<UBackgroundBlur>(UBackgroundBlur::StaticClass(), TEXT("Blur"));
    Blur->SetBlurStrength(BlurStrength);
    auto Dim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Dim"));
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
    auto Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Panel"));
    Panel->SetBrushColor(PanelColor);
    Panel->SetPadding(FMargin(32, 28));
    Frame->SetContent(Panel);

    auto Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
    Panel->SetContent(Column);

    UTextBlock* Title = MakeText(SkillTitle, TitleFont, TextColor);
    Title->SetJustification(ETextJustify::Center);
    Title->SetAutoWrapText(true);
    if (UVerticalBoxSlot* TitleSlot = Column->AddChildToVerticalBox(Title))
        TitleSlot->SetHorizontalAlignment(HAlign_Center);

    if (!Details.IsEmpty())
    {
        UTextBlock* DetailsText = MakeText(Details, DetailsFont, FLinearColor(1, 1, 1, .85f));
        DetailsText->SetJustification(ETextJustify::Center);
        DetailsText->SetAutoWrapText(true);
        if (UVerticalBoxSlot* DetailsSlot = Column->AddChildToVerticalBox(DetailsText))
        {
            DetailsSlot->SetHorizontalAlignment(HAlign_Center);
            DetailsSlot->SetPadding(FMargin(0, 12, 0, 0));
        }
    }

    auto Actions = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Actions"));
    AcceptButton = MakeButton(StyleTemplate, MakeText(FText::FromString(TEXT("Upgrade")), ButtonFont, TextColor), TEXT("AcceptButton"));
    CancelButton = MakeButton(StyleTemplate, MakeText(FText::FromString(TEXT("Cancel")), ButtonFont, TextColor), TEXT("CancelButton"));
    AcceptButton->OnClicked.AddDynamic(this, &USkillUpgradeConfirmationWidget::HandleAccept);
    CancelButton->OnClicked.AddDynamic(this, &USkillUpgradeConfirmationWidget::HandleCancel);
    for (UButton* Button : { AcceptButton.Get(), CancelButton.Get() })
    {
        if (UHorizontalBoxSlot* ActionSlot = Actions->AddChildToHorizontalBox(Button))
        {
            ActionSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            ActionSlot->SetPadding(FMargin(8, 0));
        }
    }
    if (UVerticalBoxSlot* ActionsSlot = Column->AddChildToVerticalBox(Actions))
        ActionsSlot->SetPadding(FMargin(0, 28, 0, 12));

    // The upgrade was asked for explicitly, so it gets the initial focus (A twice upgrades)
    RegisterFocusableButton(AcceptButton);
    RegisterFocusableButton(CancelButton);
    FocusButton(AcceptButton);

    UTextBlock* Hint = MakeText(FText::FromString(TEXT("Enter / A - upgrade     Esc / B - cancel")), HintFont, FLinearColor(1, 1, 1, .6f));
    if (UVerticalBoxSlot* HintSlot = Column->AddChildToVerticalBox(Hint))
        HintSlot->SetHorizontalAlignment(HAlign_Center);
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
