// Fill out your copyright notice in the Description page of Project Settings.


#include "Skills/AimingSpeed/AimingSpeedSkillUpgrade.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"


UAimingSpeedSkillUpgrade::UAimingSpeedSkillUpgrade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    Index = 0;
    BaseDescription = FText::FromString(TEXT("Increase {0} times (from the original)."));
}

bool UAimingSpeedSkillUpgrade::UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;
    const TArray<float>& Abilities = Info.SkillsProgression->AimingSpeed;
    if (!Abilities.IsValidIndex(Index)) return false;
    Info.SkillsSystem->RanksProgression.Aiming = Index;
    return true;
}

bool UAimingSpeedSkillUpgrade::IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;
    const int32 Current = Info.SkillsSystem->RanksProgression.Aiming;
    return Current >= Index;
}

FText UAimingSpeedSkillUpgrade::GetDescription_Implementation(AShooterCharacter* ShooterCharacter)
{
    const FString ToReplace = TEXT("{0}");
    FString Description = BaseDescription.ToString();
    if (!Description.Contains(ToReplace)) return BaseDescription;
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return BaseDescription;

    const TArray<float>& Abilities = Info.SkillsProgression->AimingSpeed;
    if (!(Abilities.IsValidIndex(Index) && Abilities.IsValidIndex(0))) return BaseDescription;
    const int32 BaseValue = Abilities[0];
    if (BaseValue == 0) return BaseDescription;
    const float IncreaseRate = Abilities[Index] / BaseValue;

    Description.ReplaceInline(*ToReplace, *FString::SanitizeFloat(IncreaseRate), ESearchCase::CaseSensitive);
    return FText::FromString(Description);
}
