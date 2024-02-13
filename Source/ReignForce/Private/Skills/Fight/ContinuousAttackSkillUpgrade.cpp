// Fill out your copyright notice in the Description page of Project Settings.


#include "Skills/Fight/ContinuousAttackSkillUpgrade.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"


UContinuousAttackSkillUpgrade::UContinuousAttackSkillUpgrade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    WeaponType = EWeaponType::None;
    BaseDescription = FText::FromString(TEXT("Unlock continuous attack."));
}

bool UContinuousAttackSkillUpgrade::UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;

    const TMap<EWeaponType, ESkillUnlockAbility>& ContinuousAttacks = Info.SkillsProgression->ContinuousAttacks;
    const ESkillUnlockAbility* AbilityPtr = ContinuousAttacks.Find(WeaponType);
    if (!AbilityPtr) return false;
    ESkillUnlockAbility Ability = *AbilityPtr;
    if (Ability != ESkillUnlockAbility::FromFalseToTrue) return false;

    bool* Mode = Info.SkillsSystem->RanksProgression.ContiniousAttackModes.Find(WeaponType);
    if (!Mode) return false;
    *Mode = true;
    return true;
}

bool UContinuousAttackSkillUpgrade::IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;

    const TMap<EWeaponType, ESkillUnlockAbility>& ContinuousAttacks = Info.SkillsProgression->ContinuousAttacks;
    const ESkillUnlockAbility* AbilityPtr = ContinuousAttacks.Find(WeaponType);
    if (!AbilityPtr) return false;
    ESkillUnlockAbility Ability = *AbilityPtr;
    bool bShouldBeTrue = Ability == ESkillUnlockAbility::FromFalseToTrue || Ability == ESkillUnlockAbility::AlwaysTrue;
    if (!bShouldBeTrue) return false;

    bool* Mode = Info.SkillsSystem->RanksProgression.ContiniousAttackModes.Find(WeaponType);
    return Mode && *Mode;
}

FText UContinuousAttackSkillUpgrade::GetDescription_Implementation(AShooterCharacter* ShooterCharacter)
{
    switch (WeaponType)
    {
        case EWeaponType::Unarmed: return FText::FromString(TEXT("Unarmed continuous attack."));
        case EWeaponType::Melee: return FText::FromString(TEXT("Melee continuous attack."));
        case EWeaponType::Pistol: return FText::FromString(TEXT("Pistol continuous attack."));
        default: return BaseDescription;
    }
}
