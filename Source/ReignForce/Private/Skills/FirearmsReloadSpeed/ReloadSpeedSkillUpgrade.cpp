// Fill out your copyright notice in the Description page of Project Settings.


#include "Skills/FirearmsReloadSpeed/ReloadSpeedSkillUpgrade.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"
#include "Weapons/Components/WeaponSlotsSystem.h"


UReloadSpeedSkillUpgrade::UReloadSpeedSkillUpgrade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    Index = 0;
    BaseDescription = FText::FromString(TEXT("Increase {0} times (from the original)."));
}

bool UReloadSpeedSkillUpgrade::UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;

    const TArray<float>& WeaponReloadSpeed = Info.SkillsProgression->WeaponReloadSpeed;
    if (!WeaponReloadSpeed.IsValidIndex(Index)) return false;

    Info.SkillsSystem->RanksProgression.WeaponReload = Index;
    return true;
}

bool UReloadSpeedSkillUpgrade::IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;
    const int32 Current = Info.SkillsSystem->RanksProgression.WeaponReload;
    return Current >= Index;
}

FText UReloadSpeedSkillUpgrade::GetDescription_Implementation(AShooterCharacter* ShooterCharacter)
{
    const FString ToReplace = TEXT("{0}");
    FString Description = BaseDescription.ToString();
    if (!Description.Contains(ToReplace)) return BaseDescription;
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return BaseDescription;

    const TArray<float>& WeaponReloadSpeed = Info.SkillsProgression->WeaponReloadSpeed;
    if (!(WeaponReloadSpeed.IsValidIndex(Index) && WeaponReloadSpeed.IsValidIndex(0))) return BaseDescription;
    const int32 BaseValue = WeaponReloadSpeed[0];
    if (BaseValue == 0) return BaseDescription;
    const float IncreaseRate = WeaponReloadSpeed[Index] / BaseValue;

    Description.ReplaceInline(*ToReplace, *FString::SanitizeFloat(IncreaseRate), ESearchCase::CaseSensitive);
    return FText::FromString(Description);
}

bool UReloadSpeedSkillUpgrade::CanUpgrade_Implementation(AShooterCharacter* ShooterCharacter)
{
    bool bCanUpgrade = Super::CanUpgrade_Implementation(ShooterCharacter);
    if (!(bCanUpgrade && IsValid(ShooterCharacter))) return false;

    UShooterSkillsSystem* SkillsSystem = ShooterCharacter->GetSkillsSystem();
    if (!IsValid(SkillsSystem)) return false;

    for (const auto& Slot : SkillsSystem->AvailableSlots)
    {
        if (UWeaponSlotsSystem::IsFirearmsType(Slot)) return true;
    }

    return false;
}
