// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/WeaponType.h"
#include "ShooterCharacter/Stats/PointsAccumulator.h"
#include "ShooterStatsComponent.generated.h"


USTRUCT(BlueprintType)
struct FShooterHealth
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	float MaxHealth = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	float CurrentHealth = 0;
};

USTRUCT(BlueprintType)
struct FShooterAmmo
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 CurrentAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 MaxAmount = 0;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REIGNFORCE_API UShooterStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UShooterStatsComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	#pragma region HEALTH

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	bool bHealthRegeneratorEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (EditCondition = "bHealthRegeneratorEnabled", EditConditionHides))
	FPointsAccumulator HealthRegenerator;

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetCurrentHealth() const { return Health.CurrentHealth; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetMaxHealth() const { return Health.MaxHealth; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetCurrentHealthPercent() const { return GetCurrentHealth() / GetMaxHealth(); }

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool HasNoHealth() const { return FMath::IsNearlyZero(Health.CurrentHealth); }

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool HasMaxHealth() const { return Health.CurrentHealth == Health.MaxHealth; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void RefillHealthToMax() { Health.CurrentHealth = Health.MaxHealth; }

	UFUNCTION(BlueprintCallable)
	bool AddHealthPoints(float Value);

	UFUNCTION(BlueprintCallable)
	bool SubstractHealthPoints(float Value);

	UFUNCTION(BlueprintCallable)
	void SetMaxHealth(float Value, bool bRefillCurrent = false);

	UFUNCTION(BlueprintCallable)
	bool TakeDamage(float Damage);

	#pragma endregion

	#pragma region AMMO

	UFUNCTION(BlueprintPure)
	int32 GetMaxAmmoCount(EWeaponType Weapon) const;

	UFUNCTION(BlueprintPure)
	int32 GetAmmoCount(EWeaponType Weapon) const;

	UFUNCTION(BlueprintPure)
	bool IsAmmoFull(EWeaponType Weapon) const;

	UFUNCTION(BlueprintPure)
	bool IsAmmoEmpty(EWeaponType Weapon) const;

	UFUNCTION(BlueprintCallable)
	bool AddAmmoTo(EWeaponType Weapon, int32 Amount);

	UFUNCTION(BlueprintCallable)
	bool SetWeaponAmmo(EWeaponType Weapon, int32 Amount);

	UFUNCTION(BlueprintCallable)
	bool SetWeaponMaxAmmo(EWeaponType Weapon, int32 Amount);

	UFUNCTION(BlueprintCallable)
	bool AddWeaponAmmo(EWeaponType Weapon, int32 Amount, bool bFull = true);

	UFUNCTION(BlueprintCallable)
	bool RemoveWeaponAmmo(EWeaponType Weapon);

	UFUNCTION(BlueprintCallable)
	int32 GetAmmoAsRest(EWeaponType Weapon, int32 Amount);

	#pragma endregion

	#pragma region WEAPONS
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TMap<EWeaponType, int32> WeaponModels;

	UFUNCTION(BlueprintPure)
	int32 GetWeaponModelOrDefault(EWeaponType WeaponType, int32 Default = 0) const;

	UFUNCTION(BlueprintCallable)
	void SetWeaponModelOf(EWeaponType Weapon, int32 Value);

	#pragma endregion

private:
	UPROPERTY(BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = true))
	FShooterHealth Health;

	UPROPERTY(BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = true))
	TMap<EWeaponType, FShooterAmmo> WeaponsAmmo;

	float CalcHealthWithSafeBounds(float Value) const;
	float CalcAmmoWithSafeBounds(float Value, float UpperBound) const;
};
