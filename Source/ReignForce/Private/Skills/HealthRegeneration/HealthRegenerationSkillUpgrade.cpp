// Fill out your copyright notice in the Description page of Project Settings.


#include "Skills/HealthRegeneration/HealthRegenerationSkillUpgrade.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/Stats/PointsAccumulator.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"


UHealthRegenerationSkillUpgrade::UHealthRegenerationSkillUpgrade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    Index = 0;
    BaseDescription = FText::FromString(TEXT("Increase {0} times (from the original)."));
}

bool UHealthRegenerationSkillUpgrade::UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;

    const TArray<float>& HealthRegenerationSpeed = Info.SkillsProgression->HealthRegenerationSpeed;
    if (!HealthRegenerationSpeed.IsValidIndex(Index)) return false;

    Info.SkillsSystem->RanksProgression.HealthRegenerationSpeed = Index;
	Info.StatsComponent->HealthRegenerator.Speed = Info.SkillsSystem->GetHealthRegenerationSpeedOrDefault(); 

    return true;
}

bool UHealthRegenerationSkillUpgrade::IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;
    const int32 Current = Info.SkillsSystem->RanksProgression.HealthRegenerationSpeed;
    return Current >= Index;
}

FText UHealthRegenerationSkillUpgrade::GetDescription_Implementation(AShooterCharacter* ShooterCharacter)
{
    const FString ToReplace = TEXT("{0}");
    FString Description = BaseDescription.ToString();
    if (!Description.Contains(ToReplace)) return BaseDescription;
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return BaseDescription;

    const TArray<float>& HealthRegenerationSpeed = Info.SkillsProgression->HealthRegenerationSpeed;
    if (!(HealthRegenerationSpeed.IsValidIndex(Index) && HealthRegenerationSpeed.IsValidIndex(0))) return BaseDescription;
    const int32 BaseValue = HealthRegenerationSpeed[0];
    if (BaseValue == 0) return BaseDescription;
    const float IncreaseRate = HealthRegenerationSpeed[Index] / BaseValue;

    Description.ReplaceInline(*ToReplace, *FString::SanitizeFloat(IncreaseRate), ESearchCase::CaseSensitive);
    return FText::FromString(Description);
}

