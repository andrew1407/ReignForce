// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WeaponItemWidget.h"
#include "Components/Image.h"


UWeaponItemWidget::UWeaponItemWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    InactiveOpacity = .4f;
}

void UWeaponItemWidget::SetWeaponSelected(bool bState)
{
    if (!IsValid(WeaponIcon)) return;
    FLinearColor IconColor = FLinearColor::White;
    constexpr float MaxOpacity = 1;
    IconColor.A = bState ? MaxOpacity : InactiveOpacity;
    WeaponIcon->SetBrushTintColor(IconColor);
}

void UWeaponItemWidget::SetItemVisible(bool bState)
{
    if (!IsValid(WeaponIcon)) return;
    FSlateBrush Brush = WeaponIcon->GetBrush();
    Brush.DrawAs = bState ? ESlateBrushDrawType::Image : ESlateBrushDrawType::NoDrawType;
    WeaponIcon->SetBrush(Brush);
}
