// Fill out your copyright notice in the Description page of Project Settings.


#include "Skills/Firearms/UnlockSaveAmmoModeSkillUpgrade.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"


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
