// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter/Stats/ShooterStatsComponent.h"

// Sets default values for this component's properties
UShooterStatsComponent::UShooterStatsComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bHealthRegeneratorEnabled = false;
}

bool UShooterStatsComponent::AddHealthPoints(float Value)
{
	bool bShouldAdd = !HasMaxHealth();
	if (bShouldAdd)
		Health.CurrentHealth = CalcHealthWithSafeBounds(Health.CurrentHealth + Value);
	return bShouldAdd;
}

bool UShooterStatsComponent::SubstractHealthPoints(float Value)
{
	bool bShouldSubstract = !HasNoHealth();
	if (bShouldSubstract)
		Health.CurrentHealth = CalcHealthWithSafeBounds(Health.CurrentHealth - Value);
	return bShouldSubstract;
}

void UShooterStatsComponent::SetMaxHealth(float Value, bool bRefillCurrent)
{
	Health.MaxHealth = Value;
	if (bRefillCurrent) RefillHealthToMax();
}

bool UShooterStatsComponent::TakeDamage(float Damage)
{
	bool bHasHealth = !HasNoHealth();
	if (bHasHealth)
		Health.CurrentHealth = CalcHealthWithSafeBounds(Health.CurrentHealth - Damage);
	return bHasHealth;
}

int32 UShooterStatsComponent::GetMaxAmmoCount(EWeaponType Weapon) const
{
	const FShooterAmmo* Ammo = WeaponsAmmo.Find(Weapon);
	return Ammo ? Ammo->MaxAmount : 0;
}

int32 UShooterStatsComponent::GetAmmoCount(EWeaponType Weapon) const
{
	const FShooterAmmo* Ammo = WeaponsAmmo.Find(Weapon);
	return Ammo ? Ammo->CurrentAmount : 0;
}

bool UShooterStatsComponent::IsAmmoFull(EWeaponType Weapon) const
{
	const FShooterAmmo* Ammo = WeaponsAmmo.Find(Weapon);
	return Ammo ? Ammo->CurrentAmount == Ammo->MaxAmount : false;

}

bool UShooterStatsComponent::IsAmmoEmpty(EWeaponType Weapon) const
{
	const FShooterAmmo* Ammo = WeaponsAmmo.Find(Weapon);
	return Ammo ? Ammo->CurrentAmount == 0 : true;
}

bool UShooterStatsComponent::AddAmmoTo(EWeaponType Weapon, int32 Amount)
{
	FShooterAmmo* Ammo = WeaponsAmmo.Find(Weapon);
	if (!Ammo || IsAmmoFull(Weapon)) return false;
	Ammo->CurrentAmount = CalcAmmoWithSafeBounds(Ammo->CurrentAmount + Amount, Ammo->MaxAmount);
	return true;
}

bool UShooterStatsComponent::SetWeaponAmmo(EWeaponType Weapon, int32 Amount)
{
	FShooterAmmo* Ammo = WeaponsAmmo.Find(Weapon);
	if (Ammo) Ammo->CurrentAmount = CalcAmmoWithSafeBounds(Amount, Ammo->MaxAmount);
	return !!Ammo;
}

bool UShooterStatsComponent::SetWeaponMaxAmmo(EWeaponType Weapon, int32 Amount)
{
	FShooterAmmo* Ammo = WeaponsAmmo.Find(Weapon);
	if (Ammo) Ammo->MaxAmount = Amount;
	return !!Ammo;
}

bool UShooterStatsComponent::AddWeaponAmmo(EWeaponType Weapon, int32 Amount, bool IsFull)
{
	if (WeaponsAmmo.Contains(Weapon)) return false;
	WeaponsAmmo.Add(Weapon, { Amount, IsFull ? Amount : 0 });
	return true;
}

bool UShooterStatsComponent::RemoveWeaponAmmo(EWeaponType Weapon)
{
	return WeaponsAmmo.Remove(Weapon) > 0;
}

int32 UShooterStatsComponent::GetAmmoAsRest(EWeaponType Weapon, int32 Amount)
{
	FShooterAmmo* Ammo = WeaponsAmmo.Find(Weapon);
	if (!Ammo || IsAmmoEmpty(Weapon)) return 0;
	int32 Left = Ammo->CurrentAmount - Amount;
	bool IsEnough = Left >= 0;
	Ammo->CurrentAmount = IsEnough ? Left : 0;
	return IsEnough ? Amount : Amount + Left;
}

int32 UShooterStatsComponent::GetWeaponModelOrDefault(EWeaponType WeaponType, int32 Default) const
{
	const int32* Model = WeaponModels.Find(WeaponType);
	return Model ? *Model : Default;
}

void UShooterStatsComponent::SetWeaponModelOf(EWeaponType Weapon, int32 Value)
{
	WeaponModels.FindOrAdd(Weapon, Value);
}

float UShooterStatsComponent::CalcHealthWithSafeBounds(float Value) const
{
	return FMath::Clamp(Value, 0, Health.MaxHealth);
}

float UShooterStatsComponent::CalcAmmoWithSafeBounds(float Value, float UpperBound) const
{
	return FMath::Clamp(Value, 0, UpperBound);
}
