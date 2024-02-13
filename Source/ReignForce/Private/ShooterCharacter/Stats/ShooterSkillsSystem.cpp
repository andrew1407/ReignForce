// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"

namespace
{
	constexpr auto DEFAULT_AIM_MODE = EAimAvailability::NoAim;
	constexpr float DEFAULT_ATTACK_HIT_CHANCE = 1;

	const FVector2D DEFAILT_RECOIL_MODIFIER = { 1, 1 };
	const FProjectileSpread DEFAILT_PROJECTILE_SPREAD = { 0, 0 };
}

// Sets default values for this component's properties
UShooterSkillsSystem::UShooterSkillsSystem(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	AvailableSlots = { EWeaponType::Unarmed };
}

void UShooterSkillsSystem::InitializeDefaultRanks()
{
	if (!IsProgressionAssetValid()) return;
	const ESkillUnlockAbility& FirearmsAutoReload = ProgressionAsset->FirearmsAutoReload;
	RanksProgression.bWeaponAutoReloadMode = FirearmsAutoReload == ESkillUnlockAbility::AlwaysTrue || FirearmsAutoReload == ESkillUnlockAbility::FromTrueToFalse;
	const ESkillUnlockAbility& AmmoSaveMode = ProgressionAsset->AmmoSaveMode;
	RanksProgression.bSaveAmmoMode = AmmoSaveMode == ESkillUnlockAbility::AlwaysTrue || AmmoSaveMode == ESkillUnlockAbility::FromTrueToFalse;

	for (const auto& [Key, Value] : ProgressionAsset->ContinuousAttacks)
	{
		bool bAttackMode = Value == ESkillUnlockAbility::AlwaysTrue || Value == ESkillUnlockAbility::FromTrueToFalse;
		RanksProgression.ContiniousAttackModes.FindOrAdd(Key, bAttackMode);
	}
	
	for (const auto& [Key, _] : ProgressionAsset->WeaponAttacks)
		RanksProgression.WeaponAttacks.FindOrAdd(Key, 0);

	for (const auto& [Key, _] : ProgressionAsset->AmmoStats)
		RanksProgression.AmmoCapaticy.FindOrAdd(Key, 0);

	for (const auto& [Key, _] : ProgressionAsset->WeaponRecoilModifiers)
		RanksProgression.WeaponRecoilModifiers.FindOrAdd(Key, 0);
	
	for (const auto& [Key, _] : ProgressionAsset->MeleeAttackHitChances)
		RanksProgression.MeleeAttackHitChances.FindOrAdd(Key, 0);
	
	for (const auto& [Key, _] : ProgressionAsset->FirearmsProjectileSpreads)
		RanksProgression.FirearmsProjectileSpreads.FindOrAdd(Key, 0);
}

bool UShooterSkillsSystem::IsProgressionAssetValid() const
{
	return IsValid(ProgressionAsset) && ProgressionAsset->IsValidLowLevel();
}

bool UShooterSkillsSystem::GetContituousAttackOrDefault(EWeaponType WeaponType, bool bDefault) const
{
	if (!IsProgressionAssetValid()) return bDefault;
	const bool* IsContinuous = RanksProgression.ContiniousAttackModes.Find(WeaponType);
	return IsContinuous ? *IsContinuous : false;
}

FWeaponSkillDescription UShooterSkillsSystem::GetWeaponAttackParamsOrDefault(EWeaponType WeaponType) const
{
	if (!IsProgressionAssetValid()) return {};
	FWeaponSkillDescriptionCollection* WeaponAttacks = ProgressionAsset->WeaponAttacks.Find(WeaponType);
	const int32* Rank = RanksProgression.WeaponAttacks.Find(WeaponType);
	if (!(WeaponAttacks && Rank)) return {};
	const TArray<FWeaponSkillDescription>& Collection = WeaponAttacks->Collection;
	if (!Collection.IsValidIndex(*Rank)) return {};
	return Collection[*Rank];
}

float UShooterSkillsSystem::GetAimingRateOrDefault(float Default) const
{
	if (!IsProgressionAssetValid()) return Default;
	const TArray<float>& AimingSpeed = ProgressionAsset->AimingSpeed;
	const int32& Rank = RanksProgression.Aiming;
	return AimingSpeed.IsValidIndex(Rank) ? AimingSpeed[Rank] : Default;
}

float UShooterSkillsSystem::GetSwappingRateOrDefault(float Default) const
{
	if (!IsProgressionAssetValid()) return Default;
	const TArray<float>& WeaponAttacks = ProgressionAsset->WeaponSwappingSpeed;
	const int32& Rank = RanksProgression.WeaponSwap;
	return WeaponAttacks.IsValidIndex(Rank) ? WeaponAttacks[Rank] : Default;
}

float UShooterSkillsSystem::GetWeaponReloadRateOrDefault(float Default) const
{
	if (!IsProgressionAssetValid()) return Default;
	const TArray<float>& ReloadRates = ProgressionAsset->WeaponReloadSpeed;
	const int32& Rank = RanksProgression.WeaponReload;
	return ReloadRates.IsValidIndex(Rank) ? ReloadRates[Rank] : Default;
}

int32 UShooterSkillsSystem::GetAmmoCapacityOrDefault(EWeaponType WeaponType, float Default) const
{
	if (!IsProgressionAssetValid()) return Default;
	const FAmmoCapacity* AmmoCapacity  = ProgressionAsset->AmmoStats.Find(WeaponType);
	const int32* Rank = RanksProgression.AmmoCapaticy.Find(WeaponType);
	if (!(AmmoCapacity && Rank)) return Default;
	const TArray<int32>& Ammo = AmmoCapacity->MaxCount;
	return Ammo.IsValidIndex(*Rank) ? Ammo[*Rank] : Default;
}

int32 UShooterSkillsSystem::GetHealthOrDefault(int32 Default) const
{
	if (!IsProgressionAssetValid()) return Default;
	const int32& Rank = RanksProgression.Health;
	const TArray<int32>& HealthArray = ProgressionAsset->MaxHealth;
	return HealthArray.IsValidIndex(Rank) ? HealthArray[Rank] : Default;
}

float UShooterSkillsSystem::GetHealthRegenerationSpeedOrDefault(float Default) const
{
	if (!IsProgressionAssetValid()) return Default;
	const int32& Rank = RanksProgression.HealthRegenerationSpeed;
	const TArray<float>& SpeedArray = ProgressionAsset->HealthRegenerationSpeed;
	return SpeedArray.IsValidIndex(Rank) ? SpeedArray[Rank] : Default;
}

FVector2D UShooterSkillsSystem::GetRecoilModifierOrDefault(EWeaponType WeaponType) const
{
	if (!IsProgressionAssetValid()) return DEFAILT_RECOIL_MODIFIER;
	const FRecoilModifiersCollection* Container = ProgressionAsset->WeaponRecoilModifiers.Find(WeaponType);
	const int32* Rank = RanksProgression.WeaponRecoilModifiers.Find(WeaponType);
	if (!(Container && Rank)) return DEFAILT_RECOIL_MODIFIER;
	const TArray<FVector2D>& Recoils = Container->Collection;
	return Recoils.IsValidIndex(*Rank) ? Recoils[*Rank] : DEFAILT_RECOIL_MODIFIER;
}

EAimAvailability UShooterSkillsSystem::GetAimAvailabilityOrDefault() const
{
	if (!IsProgressionAssetValid()) return DEFAULT_AIM_MODE;
	const TArray<EAimAvailability>& AimAvailability = ProgressionAsset->AimAvailability;
	const int32& Rank = RanksProgression.AimAvailability;
	return AimAvailability.IsValidIndex(Rank) ? AimAvailability[Rank] : DEFAULT_AIM_MODE;
}

float UShooterSkillsSystem::GetAttackHitChanceOrDefault(EWeaponType WeaponType) const
{
	if (!IsProgressionAssetValid()) return DEFAULT_ATTACK_HIT_CHANCE;
	const FAttackHitChancesCollection* Container = ProgressionAsset->MeleeAttackHitChances.Find(WeaponType);
	const int32* Rank = RanksProgression.MeleeAttackHitChances.Find(WeaponType);
	if (!(Container && Rank)) return DEFAULT_ATTACK_HIT_CHANCE;
	const TArray<float>& HitChances = Container->Collection;
	return HitChances.IsValidIndex(*Rank) ? HitChances[*Rank] : DEFAULT_ATTACK_HIT_CHANCE;
}

FProjectileSpread UShooterSkillsSystem::GetFirearmsProjectileSpreadOrDefault(EWeaponType WeaponType) const
{
	if (!IsProgressionAssetValid()) return DEFAILT_PROJECTILE_SPREAD;
	const FProjectileSpreadsCollection* Container = ProgressionAsset->FirearmsProjectileSpreads.Find(WeaponType);
	const int32* Rank = RanksProgression.FirearmsProjectileSpreads.Find(WeaponType);
	if (!(Container && Rank)) return DEFAILT_PROJECTILE_SPREAD;
	const TArray<FProjectileSpread>& HitChances = Container->Collection;
	return HitChances.IsValidIndex(*Rank) ? HitChances[*Rank] : DEFAILT_PROJECTILE_SPREAD;
}
