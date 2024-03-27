// Fill out your copyright notice in the Description page of Project Settings.


#include "Skills/WeaponSwapSpeed/WeaponSwapSpeedSkillUpgrade.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"
#include "Weapons/WeaponType.h"


UWeaponSwapSpeedSkillUpgrade::UWeaponSwapSpeedSkillUpgrade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    Index = 0;
    BaseDescription = FText::FromString(TEXT("Increase {0} times (from the original)."));
}

bool UWeaponSwapSpeedSkillUpgrade::UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;
    const TArray<float>& Abilities = Info.SkillsProgression->WeaponSwappingSpeed;
    if (!Abilities.IsValidIndex(Index)) return false;
    Info.SkillsSystem->RanksProgression.WeaponSwap = Index;
    return true;
}

bool UWeaponSwapSpeedSkillUpgrade::IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;
    const int32 Current = Info.SkillsSystem->RanksProgression.WeaponSwap;
    return Current >= Index;
}

FText UWeaponSwapSpeedSkillUpgrade::GetDescription_Implementation(AShooterCharacter* ShooterCharacter)
{
    const FString ToReplace = TEXT("{0}");
    FString Description = BaseDescription.ToString();
    if (!Description.Contains(ToReplace)) return BaseDescription;
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return BaseDescription;

    const TArray<float>& Abilities = Info.SkillsProgression->WeaponSwappingSpeed;
    if (!(Abilities.IsValidIndex(Index) && Abilities.IsValidIndex(0))) return BaseDescription;
    const int32 BaseValue = Abilities[0];
    if (BaseValue == 0) return BaseDescription;
    const float IncreaseRate = Abilities[Index] / BaseValue;

    Description.ReplaceInline(*ToReplace, *FString::SanitizeFloat(IncreaseRate), ESearchCase::CaseSensitive);
    return FText::FromString(Description);
}

bool UWeaponSwapSpeedSkillUpgrade::CanUpgrade_Implementation(AShooterCharacter* ShooterCharacter)
{
    bool bCanUpgrade = Super::CanUpgrade_Implementation(ShooterCharacter);
    if (!(bCanUpgrade && IsValid(ShooterCharacter))) return false;

    UShooterSkillsSystem* SkillsSystem = ShooterCharacter->GetSkillsSystem();
    if (!IsValid(SkillsSystem)) return false;

    TSet<EWeaponType> AvailableSlots = SkillsSystem->AvailableSlots;
    AvailableSlots.Remove(EWeaponType::Unarmed);
    return !AvailableSlots.IsEmpty();
}
