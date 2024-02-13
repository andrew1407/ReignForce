// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AimWidget.h"
#include "Components/Image.h"


void UAimWidget::SetAimVisibility(bool bState)
{
    if (!IsValid(AimImage)) return;
    FSlateBrush Brush = AimImage->GetBrush();
    Brush.DrawAs = bState ? ESlateBrushDrawType::Image : ESlateBrushDrawType::NoDrawType;
    AimImage->SetBrush(Brush);
}

void UAimWidget::SetAimTargetDetected(bool bState)
{
    if (!IsValid(AimImage)) return;
    AimImage->SetBrushTintColor(bState ? FLinearColor::Red : FLinearColor::White);
}
