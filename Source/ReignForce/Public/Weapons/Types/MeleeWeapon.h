// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Types/Weapon.h"
#include "MeleeWeapon.generated.h"


class UBoxComponent;
class UAudioComponent;
class UParticleSystem;

class UCooldown;

/**
 * 
 */
UCLASS(Abstract)
class REIGNFORCE_API AMeleeWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	AMeleeWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = 0, UIMin = 0, ClampMax = 1, UIMax = 1, Units = "Seconds"))
	float HitChance;

    virtual FVector GetAimingPosition_Implementation() const override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	UFUNCTION(BlueprintCallable)
	void EnableCollision(int32 Index = 0);

	UFUNCTION(BlueprintCallable)
	void DisableCollision(int32 Index = 0);

	UFUNCTION(BlueprintCallable)
	void SetCollisionState(bool bState, int32 Index = 0);

	UFUNCTION(BlueprintPure)
	bool IsDamageCollisionEnabled(int32 Index = 0) const;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = true))
	TObjectPtr<UBoxComponent> DamageCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Sound", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> OnHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Sound", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Times"))
	float OnHitSoundVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Visual", meta = (AllowPrivateAccess = true))
	TObjectPtr<UParticleSystem> HitTargetEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Visual", meta = (AllowPrivateAccess = true))
	FVector HitTargetEffectScale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack|Cooldown", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Seconds"))
	float CooldownDuration;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UCooldown> Cooldown;

	UAudioComponent* PlayOnHitSound() const;
	UParticleSystemComponent* SpawnHitTargetEffect() const;
};
