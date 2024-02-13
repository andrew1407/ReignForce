// Fill out your copyright notice in the Description page of Project Settings.


#include "Skills/AimHelp/AimHelpSkillUpgrade.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"
#include "ShooterCharacter/AimAvailability.h"


UAimHelpSkillUpgrade::UAimHelpSkillUpgrade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    Index = 0;
    BaseDescription = FText::FromString(TEXT("Upgrade Aim Help."));
}

bool UAimHelpSkillUpgrade::UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;
    const TArray<EAimAvailability>& Abilities = Info.SkillsProgression->AimAvailability;
    if (!Abilities.IsValidIndex(Index)) return false;
    Info.SkillsSystem->RanksProgression.AimAvailability = Index;
    return true;
}

bool UAimHelpSkillUpgrade::IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;

    const int32 Current = Info.SkillsSystem->RanksProgression.AimAvailability;
    return Current >= Index;
}

FText UAimHelpSkillUpgrade::GetDescription_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return BaseDescription;

    const TArray<EAimAvailability>& Abilities = Info.SkillsProgression->AimAvailability;
    if (!Abilities.IsValidIndex(Index)) return BaseDescription;

    switch (Abilities[Index])
    {
        case EAimAvailability::AimPresent: return FText::FromString(TEXT("Aim present."));
        case EAimAvailability::AimTraceAimingOnly: return FText::FromString(TEXT("Aim trace when aiming only."));
        case EAimAvailability::AimTraceAlways: return FText::FromString(TEXT("Aim trace (full)."));
        default: return BaseDescription;
    }
}
