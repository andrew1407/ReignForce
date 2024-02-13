// Fill out your copyright notice in the Description page of Project Settings.


#include "Skills/Fight/FightAttackSkillUpgrade.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"


UFightAttackSkillUpgrade::UFightAttackSkillUpgrade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    Index = 0;
    WeaponType = EWeaponType::None;
    BaseDescription = FText::FromString(TEXT("Increase attack speed {0} times (from the original)."));
}

bool UFightAttackSkillUpgrade::UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;

    const TMap<EWeaponType, FWeaponSkillDescriptionCollection>& WeaponAttacks = Info.SkillsProgression->WeaponAttacks;
    const FWeaponSkillDescriptionCollection* Description = WeaponAttacks.Find(WeaponType);
    if (!(Description && Description->Collection.IsValidIndex(Index))) return false;

    int32* Rank = Info.SkillsSystem->RanksProgression.WeaponAttacks.Find(WeaponType);
    if (!Rank) return false;
    *Rank = Index;
    return true;
}

bool UFightAttackSkillUpgrade::IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;
    int32* CurrentPtr = Info.SkillsSystem->RanksProgression.WeaponAttacks.Find(WeaponType);
    if (!CurrentPtr) return false;
    return (*CurrentPtr) >= Index;
}

FText UFightAttackSkillUpgrade::GetDescription_Implementation(AShooterCharacter* ShooterCharacter)
{
    const FString ToReplace = TEXT("{0}");
    FString Description = BaseDescription.ToString();
    if (!Description.Contains(ToReplace)) return BaseDescription;
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return BaseDescription;

    const TMap<EWeaponType, FWeaponSkillDescriptionCollection>& WeaponAttacks = Info.SkillsProgression->WeaponAttacks;
    const FWeaponSkillDescriptionCollection* DescriptionCollection = WeaponAttacks.Find(WeaponType);
    if (!(DescriptionCollection && DescriptionCollection->Collection.IsValidIndex(Index))) return BaseDescription;
    if (!DescriptionCollection->Collection.IsValidIndex(0)) return BaseDescription;

    const float AnimationRate = DescriptionCollection->Collection[Index].AnimationRate;
    const int32 BaseRate = DescriptionCollection->Collection[0].AnimationRate;
    if (BaseRate == 0) return BaseDescription;
    const float IncreaseRate = AnimationRate / BaseRate;

    Description.ReplaceInline(*ToReplace, *FString::SanitizeFloat(IncreaseRate), ESearchCase::CaseSensitive);
    return FText::FromString(Description);
}
