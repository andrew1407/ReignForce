// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterCharacter/Stats/RanksProgression.h"
#include "Weapons/WeaponType.h"
#include "Weapons/ProjectileSpread.h"
#include "ShooterCharacter/AimAvailability.h"
#include "ShooterSkillsSystem.generated.h"


class UShooterSkillsProgression;
struct FMeleeSkillDescription;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REIGNFORCE_API UShooterSkillsSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UShooterSkillsSystem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	FRanksProgression RanksProgression;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	TSet<EWeaponType> AvailableSlots;

	UFUNCTION(BlueprintCallable)
	void InitializeDefaultRanks();

	UFUNCTION(BlueprintPure)
	bool IsProgressionAssetValid() const;

	UFUNCTION(BlueprintPure)
	bool GetContituousAttackOrDefault(EWeaponType WeaponType, bool bDefault = false) const;

	UFUNCTION(BlueprintPure)
	FWeaponSkillDescription GetWeaponAttackParamsOrDefault(EWeaponType WeaponType) const;

	UFUNCTION(BlueprintPure)
	float GetAimingRateOrDefault(float Default = 1) const;

	UFUNCTION(BlueprintPure)
	float GetSwappingRateOrDefault(float Default = 1) const;

	UFUNCTION(BlueprintPure)
	float GetWeaponReloadRateOrDefault(float Default = 1) const;

	UFUNCTION(BlueprintPure)
	int32 GetAmmoCapacityOrDefault(EWeaponType WeaponType, float Default = 0) const;

	UFUNCTION(BlueprintPure)
	int32 GetHealthOrDefault(int32 Default = 0) const;

	UFUNCTION(BlueprintPure)
	float GetHealthRegenerationSpeedOrDefault(float Default = 1) const;

	UFUNCTION(BlueprintPure)
	FVector2D GetRecoilModifierOrDefault(EWeaponType WeaponType) const;

	UFUNCTION(BlueprintPure)
	EAimAvailability GetAimAvailabilityOrDefault() const;
	
	UFUNCTION(BlueprintPure)
	float GetAttackHitChanceOrDefault(EWeaponType WeaponType) const;

	UFUNCTION(BlueprintPure)
	FProjectileSpread GetFirearmsProjectileSpreadOrDefault(EWeaponType WeaponType) const;

	FORCEINLINE UShooterSkillsProgression* GetProgressionAsset() const { return ProgressionAsset; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression", meta = (AllowPrivateAccess = true))
	TObjectPtr<UShooterSkillsProgression> ProgressionAsset;
};
