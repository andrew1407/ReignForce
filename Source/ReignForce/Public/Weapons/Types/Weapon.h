// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapons/WeaponType.h"
#include "Weapon.generated.h"


class USkeletalMeshComponent;
class UTimelineComponent;
class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDamageCaused, float, Damage, EWeaponType, WeaponType, AActor*, HitTarget, int32, Index);

UCLASS(Abstract)
class REIGNFORCE_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(BlueprintAssignable)
	FOnDamageCaused OnDamageCaused;

	UFUNCTION(BlueprintNativeEvent, Category = "Character")
    FVector GetAimingPosition() const;

    virtual FVector GetAimingPosition_Implementation() const { return FVector::ZeroVector; }

	FORCEINLINE EWeaponType GetType() const { return Type; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetDistance() const { return Distance; }
	FORCEINLINE USkeletalMeshComponent* GetMesh() const { return SkeletalMeshComponent; }

	UFUNCTION(BlueprintPure)
	bool Aim(FHitResult& HitResult, const FVector& Direction = FVector::ForwardVector) const;

	UFUNCTION(BlueprintCallable)
	bool CheckAimTarget(AActor* Target) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	EWeaponType Type;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = 0, UIMin = 0))
	float Damage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = 0, UIMin = 0))
	float Distance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Noise", meta = (ClampMin = 0, UIMin = 0, Units = "Times"))
	float Loudness;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Noise", meta = (ClampMin = 0, UIMin = 0))
	float LoundnessRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Mesh")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Aims", meta = (AllowPrivateAccess = true))
	TSet<TSubclassOf<AActor>> AimsToHit;
};
