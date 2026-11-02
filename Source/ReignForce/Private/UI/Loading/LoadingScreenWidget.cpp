// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Loading/LoadingScreenWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Styling/CoreStyle.h"
#include "Engine/Texture2D.h"

#include "Components/CircularThrobber.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScaleBox.h"
#include "CommonTextBlock.h"


void ULoadingScreenWidget::SetImage(UTexture2D* Image)
{
    BackgroundImage = Image;
    auto ImageWidget = IsValid(WidgetTree) ? WidgetTree->FindWidget<UImage>(TEXT("Background")) : nullptr;
    if (IsValid(ImageWidget) && IsValid(Image)) ImageWidget->SetBrushFromTexture(Image, true);
}

void ULoadingScreenWidget::SetInvisible(bool bInInvisible)
{
    bInvisible = bInInvisible;
    ApplyInvisible();
}

void ULoadingScreenWidget::ApplyInvisible()
{
    if (!IsValid(WidgetTree)) return;
    // The cover stays hit testable (fully transparent, not hidden), so the menus under it don't get the clicks
    if (auto Background = WidgetTree->FindWidget<UImage>(TEXT("Background")))
        Background->SetRenderOpacity(bInvisible ? 0.f : 1.f);
    if (UWidget* Indicator = WidgetTree->FindWidget(TEXT("Indicator")))
        Indicator->SetVisibility(bInvisible ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}

TSharedRef<SWidget> ULoadingScreenWidget::RebuildWidget()
{
    if (!bBuilt) Build();
    return Super::RebuildWidget();
}

void ULoadingScreenWidget::Build()
{
    if (!IsValid(WidgetTree)) return;
    bBuilt = true;

    auto Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
    WidgetTree->RootWidget = Root;

    // Image covers the whole screen keeping its aspect ratio
    auto Scale = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), TEXT("BackgroundScale"));
    Scale->SetStretch(EStretch::ScaleToFill);
    auto Image = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Background"));
    if (IsValid(BackgroundImage)) Image->SetBrushFromTexture(BackgroundImage, true);
    else Image->SetColorAndOpacity(FLinearColor::Black);
    Scale->SetContent(Image);
    if (UOverlaySlot* ScaleSlot = Root->AddChildToOverlay(Scale))
    {
        ScaleSlot->SetHorizontalAlignment(HAlign_Fill);
        ScaleSlot->SetVerticalAlignment(VAlign_Fill);
    }

    auto Indicator = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Indicator"));
    auto Throbber = WidgetTree->ConstructWidget<UCircularThrobber>(UCircularThrobber::StaticClass(), TEXT("Throbber"));
    Throbber->SetRadius(18);
    auto Text = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("LoadingText"));
    Text->SetText(FText::FromString(TEXT("Loading...")));
    Text->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 28));
    Text->SetShadowOffset(FVector2D(2));
    Text->SetShadowColorAndOpacity(FLinearColor(0, 0, 0, .8f));
    if (UHorizontalBoxSlot* ThrobberSlot = Indicator->AddChildToHorizontalBox(Throbber))
        ThrobberSlot->SetVerticalAlignment(VAlign_Center);
    if (UHorizontalBoxSlot* TextSlot = Indicator->AddChildToHorizontalBox(Text))
    {
        TextSlot->SetVerticalAlignment(VAlign_Center);
        TextSlot->SetPadding(FMargin(16, 0, 0, 0));
    }
    if (UOverlaySlot* IndicatorSlot = Root->AddChildToOverlay(Indicator))
    {
        IndicatorSlot->SetHorizontalAlignment(HAlign_Right);
        IndicatorSlot->SetVerticalAlignment(VAlign_Bottom);
        IndicatorSlot->SetPadding(FMargin(48));
    }
    ApplyInvisible();
}
