// Fill out your copyright notice in the Description page of Project Settings.


#include "Skills/HealthPoints/HealthPointsSkillUpgrade.h"

#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/PointsModifierType.h"

#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"


UHealthPointsSkillUpgrade::UHealthPointsSkillUpgrade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    Index = 0;
    BaseDescription = FText::FromString(TEXT("Increase to {0} points."));
}

bool UHealthPointsSkillUpgrade::UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;

    const TArray<int32>& MaxHealth = Info.SkillsProgression->MaxHealth;
    if (!MaxHealth.IsValidIndex(Index)) return false;

    Info.SkillsSystem->RanksProgression.Health = Index;

    const float HealthBeforeUpgrade = ShooterCharacter->GetHealthPoints();
	constexpr bool bSetFullHealth = true;
    Info.StatsComponent->SetMaxHealth(Info.SkillsSystem->GetHealthOrDefault(), bSetFullHealth);
    const float HealthAfterUpgrade = ShooterCharacter->GetHealthPoints();
    const float Diff = HealthAfterUpgrade - HealthBeforeUpgrade;
	ShooterCharacter->OnHealthChanged.Broadcast(Diff, HealthAfterUpgrade, HealthBeforeUpgrade, EPointsModifierType::SkillUpgrade);

    return true;
}

bool UHealthPointsSkillUpgrade::IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;
    const int32 Current = Info.SkillsSystem->RanksProgression.Health;
    return Current >= Index;
}

FText UHealthPointsSkillUpgrade::GetDescription_Implementation(AShooterCharacter* ShooterCharacter)
{
    const FString ToReplace = TEXT("{0}");
    FString Description = BaseDescription.ToString();
    if (!Description.Contains(ToReplace)) return BaseDescription;
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return BaseDescription;

    const TArray<int32>& MaxHealth = Info.SkillsProgression->MaxHealth;
    if (!MaxHealth.IsValidIndex(Index)) return BaseDescription;
    const int32 Health = MaxHealth[Index];

    Description.ReplaceInline(*ToReplace, *FString::FromInt(Health), ESearchCase::CaseSensitive);
    return FText::FromString(Description);
}
