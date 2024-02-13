// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ShooterStatsWidget.h"
#include "UI/WeaponItemWidget.h"
#include "UI/FirearmsItemWidget.h"
#include "UI/HealthBarWidget.h"
#include "UI/Skills/SkillsCountWidget.h"


UShooterStatsWidget::UShooterStatsWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    WeaponSlots =
    {
        EWeaponType::Unarmed,
        EWeaponType::Melee,
        EWeaponType::Pistol,
        EWeaponType::Rifle,
    };
}

UWeaponItemWidget* UShooterStatsWidget::GetWeaponItemOf(EWeaponType WeaponType) const
{
    switch (WeaponType)
    {
        case EWeaponType::Unarmed: return UnarmedItem;
        case EWeaponType::Melee: return MeleeItem;
        case EWeaponType::Pistol: return PistolItem;
        case EWeaponType::Rifle: return RifleItem;
    }
    return nullptr;
}

UFirearmsItemWidget* UShooterStatsWidget::GetFirearmsItemOf(EWeaponType WeaponType) const
{
    switch (WeaponType)
    {
        case EWeaponType::Pistol: return PistolItem;
        case EWeaponType::Rifle: return RifleItem;
    }
    return nullptr;
}

void UShooterStatsWidget::SetWeaponItemVisibility(EWeaponType WeaponType, bool bState)
{
    UWeaponItemWidget* Widget = GetWeaponItemOf(WeaponType);
    if (IsValid(Widget)) Widget->SetItemVisible(bState);
}

void UShooterStatsWidget::SetActiveWeaponAs(EWeaponType WeaponType)
{
    for (const auto& Type : WeaponSlots)
    {
        UWeaponItemWidget* Widget = GetWeaponItemOf(Type);
        if (IsValid(Widget)) Widget->SetWeaponSelected(WeaponType == Type);
    }
}

void UShooterStatsWidget::SetAmmoTo(EWeaponType WeaponType, int32 WeaponAmmo, int32 StockAmmo)
{
    UFirearmsItemWidget* Widget = GetFirearmsItemOf(WeaponType);
    if (!IsValid(Widget)) return;
    if (WeaponAmmo >= 0) Widget->SetWeaponAmmoValue(WeaponAmmo);
    if (StockAmmo >= 0) Widget->SetAmmoStockValue(StockAmmo);
}

void UShooterStatsWidget::SetHealthValue(int32 Value, float Percent)
{
    if (IsValid(HealthBar)) HealthBar->SetHealthValue(Value, Percent);
}

void UShooterStatsWidget::SetSkillsCount(int32 Value)
{
    if (!IsValid(SkillsCount) || Value < 0) return;
    SkillsCount->SetSkillsCount(Value);
    const ESlateVisibility Buttonisibility = Value ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
    SkillsCount->SetVisibility(Buttonisibility);
}
