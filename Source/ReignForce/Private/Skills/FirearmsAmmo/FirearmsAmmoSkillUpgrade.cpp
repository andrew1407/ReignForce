// Fill out your copyright notice in the Description page of Project Settings.


#include "Skills/FirearmsAmmo/FirearmsAmmoSkillUpgrade.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"


UFirearmsAmmoSkillUpgrade::UFirearmsAmmoSkillUpgrade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    WeaponType = EWeaponType::None;
    Index = 0;
    BaseDescription = FText::FromString(TEXT("Increase capacity to {0} bullets."));
    bRefillAmmoAfterUpgrade = true;
}

bool UFirearmsAmmoSkillUpgrade::UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;

    const TMap<EWeaponType, FAmmoCapacity>& AmmoStats = Info.SkillsProgression->AmmoStats;
    const FAmmoCapacity* Ammo = AmmoStats.Find(WeaponType);
    if (!(Ammo && Ammo->MaxCount.IsValidIndex(Index))) return false;

    int32* Capaticy = Info.SkillsSystem->RanksProgression.AmmoCapaticy.Find(WeaponType);
    if (!Capaticy) return false;
    *Capaticy = Index;

    constexpr uint8 DefaultFailureValue = 0;
    const int32 AmmoCapacityUpgrade = Info.SkillsSystem->GetAmmoCapacityOrDefault(WeaponType, DefaultFailureValue);
    if (AmmoCapacityUpgrade <= DefaultFailureValue) return false;
    bool bAmmoUpgradeSet = Info.StatsComponent->SetWeaponMaxAmmo(WeaponType, AmmoCapacityUpgrade);
    if (!bAmmoUpgradeSet) return false;
    if (bRefillAmmoAfterUpgrade) ShooterCharacter->SetFullAmmoForWeapon(WeaponType);

    return true;
}

bool UFirearmsAmmoSkillUpgrade::IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;
    int32* CurrentPtr = Info.SkillsSystem->RanksProgression.AmmoCapaticy.Find(WeaponType);
    if (!CurrentPtr) return false;
    return (*CurrentPtr) >= Index;
}

FText UFirearmsAmmoSkillUpgrade::GetDescription_Implementation(AShooterCharacter* ShooterCharacter)
{
    const FString ToReplace = TEXT("{0}");
    FString Description = BaseDescription.ToString();
    if (!Description.Contains(ToReplace)) return BaseDescription;
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return BaseDescription;

    const TMap<EWeaponType, FAmmoCapacity>& AmmoStats = Info.SkillsProgression->AmmoStats;
    const FAmmoCapacity* Ammo = AmmoStats.Find(WeaponType);
    if (!(Ammo && Ammo->MaxCount.IsValidIndex(Index))) return BaseDescription;

    Description.ReplaceInline(*ToReplace, *FString::FromInt(Ammo->MaxCount[Index]), ESearchCase::CaseSensitive);
    return FText::FromString(Description);
}

bool UFirearmsAmmoSkillUpgrade::CanUpgrade_Implementation(AShooterCharacter* ShooterCharacter)
{
    bool bCanUpgrade = Super::CanUpgrade_Implementation(ShooterCharacter);
    if (!(bCanUpgrade && IsValid(ShooterCharacter))) return false;
    UShooterSkillsSystem* SkillsSystem = ShooterCharacter->GetSkillsSystem();
    return IsValid(SkillsSystem) && SkillsSystem->AvailableSlots.Contains(WeaponType);
}
