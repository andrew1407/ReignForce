// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HealthBarWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Curves/CurveLinearColor.h"


void UHealthBarWidget::SetHealthValue(int32 Value, float Percent)
{
    if (!IsValid(HealthBar)) return;
    const float ClampedValue = FMath::Clamp(Percent, 0, 1);
    HealthBar->SetPercent(ClampedValue);

    if (IsValid(NumericValue)) NumericValue->SetText(FText::AsNumber(Value));

    if (!IsValid(HealthBarColorCurve)) return;
    FProgressBarStyle HealthBarrStyle = HealthBar->GetWidgetStyle();
    FSlateBrush& BackgroundImage = HealthBarrStyle.BackgroundImage;
    FSlateBrush& FillImage = HealthBarrStyle.FillImage;

    FLinearColor FillColor = HealthBarColorCurve->GetLinearColorValue(ClampedValue);
    FLinearColor BackgroundColor = FillColor;
    FillColor.A = FillImage.TintColor.GetSpecifiedColor().A;
    BackgroundColor.A = BackgroundImage.TintColor.GetSpecifiedColor().A;

    FillImage.TintColor = FillColor;
    BackgroundImage.TintColor = BackgroundColor;
    HealthBar->SetWidgetStyle(HealthBarrStyle);
}
