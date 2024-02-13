// Fill out your copyright notice in the Description page of Project Settings.


#include "Skills/Firearms/UnlockAutoreloadSkillUpgrade.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"


UUnlockAutoreloadSkillUpgrade::UUnlockAutoreloadSkillUpgrade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    BaseDescription = FText::FromString(TEXT("Unlock weapon autoreload."));
}

bool UUnlockAutoreloadSkillUpgrade::UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;

    ESkillUnlockAbility FirearmsAutoReload = Info.SkillsProgression->FirearmsAutoReload;
    if (FirearmsAutoReload != ESkillUnlockAbility::FromFalseToTrue) return false;

    Info.SkillsSystem->RanksProgression.bWeaponAutoReloadMode = true;
    return true;
}

bool UUnlockAutoreloadSkillUpgrade::IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;

    ESkillUnlockAbility FirearmsAutoReload = Info.SkillsProgression->FirearmsAutoReload;
    bool bShouldBeTrue = FirearmsAutoReload == ESkillUnlockAbility::FromFalseToTrue || FirearmsAutoReload == ESkillUnlockAbility::AlwaysTrue;
    if (!bShouldBeTrue) return true;

    return Info.SkillsSystem->RanksProgression.bWeaponAutoReloadMode;
}
