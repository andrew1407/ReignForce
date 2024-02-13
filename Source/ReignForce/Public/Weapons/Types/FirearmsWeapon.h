// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Types/Weapon.h"
#include "Weapons/ProjectileSpread.h"
#include "FirearmsWeapon.generated.h"


class UParticleSystem;
class UParticleSystemComponent;
class UAudioComponent;

/**
 * 
 */
UCLASS(Abstract)
class REIGNFORCE_API AFirearmsWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	AFirearmsWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual FVector GetAimingPosition_Implementation() const override;

	UFUNCTION(BlueprintCallable)
	bool Fire(const FVector& Direction = FVector::ForwardVector, const FRotator& Rotation = FRotator::ZeroRotator, const FProjectileSpread& ProjectileSpread = FProjectileSpread());

	UFUNCTION(BlueprintCallable)
	int32 RefillAmmo(int32 AmmoToRefill, bool bSaveRest = false);

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetIsAmmoEmpty() const { return AmmoCount <= 0; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool GetIsAmmoFull() const { return AmmoCount == MaxAmmoCount; }

	FORCEINLINE FVector2D GetRecoil() const { return Recoil; }
	FORCEINLINE float GetShootingDistance() const { return Distance; }
	FORCEINLINE float GetRateOfFire() const { return RateOfFire; }
	FORCEINLINE int32 GetMaxAmmo() const { return MaxAmmoCount; }
	FORCEINLINE int32 GetAmmoCount() const { return AmmoCount; }
	FORCEINLINE int32 GetFreeSlots() const { return MaxAmmoCount - AmmoCount; }

private:
	#pragma region CHARACTERISTICS

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting", meta = (AllowPrivateAccess = true))
	FName MuzzlefashSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Seconds"))
	float RateOfFire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting", meta = (AllowPrivateAccess = true))
	FVector2D Recoil;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting|Capacity", meta = (AllowPrivateAccess = true))
	int32 MaxAmmoCount;

	UPROPERTY(BlueprintReadWrite, Category = "Shooting|Capacity", meta = (AllowPrivateAccess = true))
	int32 AmmoCount;

	#pragma endregion

	#pragma region VISUAL_EFFECTS

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting|Visual", meta = (AllowPrivateAccess = true))
	TObjectPtr<UParticleSystem> ShootingEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting|Visual", meta = (AllowPrivateAccess = true))
	FVector ShootingEffectScale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting|Visual", meta = (AllowPrivateAccess = true))
	TObjectPtr<UParticleSystem> HitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting|Visual", meta = (AllowPrivateAccess = true))
	FVector HitEffectScale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Visual", meta = (AllowPrivateAccess = true))
	TObjectPtr<UParticleSystem> HitTargetEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting|Visual", meta = (AllowPrivateAccess = true))
	FVector HitTargetEffectScale;

	#pragma endregion

	#pragma region AUDIO_EFFECTS

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting|Audio", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting|Audio", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Times"))
	float FireSoundVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting|Audio", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> EmptyGunShotSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting|Audio", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Times"))
	float EmptyGunShotSoundVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting|Audio", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting|Audio", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Times"))
	float HitSoundVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting|Audio", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> HitTargetSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting|Audio", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Times"))
	float HitTargetSoundVolume;

	#pragma endregion

	UParticleSystemComponent* SpawnShootEffect(const FName& Socket, const FVector& Location, const FRotator& Rotation) const;
	UParticleSystemComponent* SpawnHitEffect(const FHitResult& HitResult) const;
	UParticleSystemComponent* SpawnHitTargetEffect(const FHitResult& HitResult) const;

	UAudioComponent* PlayFireSound(const FVector& Location) const;
	UAudioComponent* PlayEmptyGunShotSound(const FVector& Location) const;
	UAudioComponent* PlayHitSound(const FHitResult& HitResult) const;
	UAudioComponent* PlayHitTargetSound(const FHitResult& HitResult) const;

	FVector AddDirectionSpread(const FVector& Direction, const FProjectileSpread& ProjectileSpread);
};
