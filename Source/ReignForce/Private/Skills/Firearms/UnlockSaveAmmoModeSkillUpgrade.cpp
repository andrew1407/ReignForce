// Fill out your copyright notice in the Description page of Project Settings.


#include "Skills/Firearms/UnlockSaveAmmoModeSkillUpgrade.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"
#include "Weapons/Components/WeaponSlotsSystem.h"


UUnlockSaveAmmoModeSkillUpgrade::UUnlockSaveAmmoModeSkillUpgrade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    BaseDescription = FText::FromString(TEXT("Save left ammo on reaload."));
}

bool UUnlockSaveAmmoModeSkillUpgrade::UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;

    ESkillUnlockAbility AmmoSaveMode = Info.SkillsProgression->AmmoSaveMode;
    if (AmmoSaveMode != ESkillUnlockAbility::FromFalseToTrue) return false;

    Info.SkillsSystem->RanksProgression.bSaveAmmoMode = true;
    return true;
}

bool UUnlockSaveAmmoModeSkillUpgrade::IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;

    ESkillUnlockAbility AmmoSaveMode = Info.SkillsProgression->AmmoSaveMode;
    bool bShouldBeTrue = AmmoSaveMode == ESkillUnlockAbility::FromFalseToTrue || AmmoSaveMode == ESkillUnlockAbility::AlwaysTrue;
    if (!bShouldBeTrue) return true;

    return Info.SkillsSystem->RanksProgression.bSaveAmmoMode;
}

bool UUnlockSaveAmmoModeSkillUpgrade::CanUpgrade_Implementation(AShooterCharacter* ShooterCharacter)
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
