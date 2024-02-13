// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/ShooterState.h"
#include "Weapons/WeaponType.h"
#include "Weapons/Types/Weapon.h"
#include "CombatSystem.generated.h"


class UTimelineComponent;
class UBoxComponent;

class USpringArmComponent;
class UCameraComponent;

class UCurveFloat;
class UAnimMontage;
class USoundBase;

class UCooldown;
class AWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnPunchDamageCaused, float, Damage, EWeaponType, WeaponType, AActor*, HitTarget, int32, Index);

USTRUCT(BlueprintType)
struct FAttackDescription
{
    GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UAnimMontage*> Animations;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REIGNFORCE_API UCombatSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatSystem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(BlueprintReadWrite, Category = "Combat|State")
	EShooterState ShooterState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacking|HitChance", meta = (ClampMin = 0, UIMin = 0, ClampMax = 1, UIMax = 1, Units = "Times"))
	float PunchHitChance;

	UPROPERTY(BlueprintAssignable)
	FOnPunchDamageCaused OnPunchDamageCaused;

	#pragma region AIMING

	UFUNCTION(BlueprintCallable)
	void ToggleAimFocus();

	UFUNCTION(BlueprintCallable)
	void SetRotationLock(bool bRotationLocked);

	UFUNCTION(BlueprintCallable)
	void RotateYawToCameraView();

	UFUNCTION(BlueprintCallable)
	void ActivateAimingAction(bool bState);

	UFUNCTION(BlueprintCallable)
	void StartAiming(float Intensity = 10);

	UFUNCTION(BlueprintCallable)
	void StopAiming();

	UFUNCTION(BlueprintPure)
	bool TraceAimWith(EWeaponType WeaponType, AWeapon* Weapon = nullptr, const FVector& Direction = FVector::ForwardVector) const;

	FORCEINLINE bool GetAimingState() const { return bAiming; }
	FORCEINLINE bool GetAimFocus() const { return bAimFocus; }
	FORCEINLINE float GetAimRecoilReducer() const { return AimRecoilReducer; }

	#pragma endregion

	#pragma region ATTACKING

	FORCEINLINE TMap<EWeaponType, TPair<FTimerHandle, bool>>& GetRateOfFireDelays() { return RateOfFireDelays; }

	UFUNCTION(BlueprintCallable)
	bool AttackWith(EWeaponType Weapon, int32 Rank = 0, float Rate = 1);

	UFUNCTION(BlueprintCallable)
	bool ReloadWeapon(EWeaponType Weapon, float Rate = 1);

	UFUNCTION(BlueprintCallable)
	void ActivateRateOfFireDelay(EWeaponType Weapon, float Delay);

	UFUNCTION(BlueprintCallable)
	void StopRateOfFireDelay(EWeaponType Weapon);

	UFUNCTION(BlueprintPure)
	bool IsWeaponDelaying(EWeaponType Weapon) const;

	#pragma endregion

	#pragma region PUNCHING

	UFUNCTION(BlueprintPure)
	UBoxComponent* GetFistCollision(int32 Index = 0) const;

	UFUNCTION(BlueprintPure)
	bool PunchAimTrace(FHitResult& HitResult, const FVector& Direction = FVector::ForwardVector) const;

	UFUNCTION(BlueprintCallable)
	void SetFistCollisionState(bool bState, int32 Index = 0);

	FORCEINLINE float GetFistDamage() const { return FistDamage; }
	FORCEINLINE TPair<UBoxComponent*, UBoxComponent*> GetFistCollisions() const { return { LeftFist.Get(), RightFist.Get() }; }
	FORCEINLINE TPair<int32, int32> GetFistIndices() const { return { LeftFistIndex, RightFistIndex }; }

	#pragma endregion

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	#pragma region COMPONENT_REFS

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aiming|Refs", meta = (AllowPrivateAccess = true))
    TObjectPtr<UCurveFloat> AimimgTimelineCurve;

	UPROPERTY(BlueprintReadOnly, Category = "Aiming|Refs", meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<UTimelineComponent> AimingTimeline;

	UPROPERTY(BlueprintReadOnly, Category = "Aiming|Refs", meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(BlueprintReadOnly, Category = "Aiming|Refs", meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(BlueprintReadOnly, Category = "Punching", meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<UBoxComponent> RightFist;

	UPROPERTY(BlueprintReadOnly, Category = "Punching", meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<UBoxComponent> LeftFist;

	#pragma endregion

	#pragma region AIMING_PRIVATE

	UPROPERTY(BlueprintReadWrite, Category = "Aiming", meta = (AllowPrivateAccess = true))
	bool bAiming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming", meta = (AllowPrivateAccess = true))
	bool bAimFocus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Times"))
	float AimRecoilReducer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aiming", meta = (AllowPrivateAccess = true))
	TSet<TSubclassOf<AActor>> AimTargets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming|Camera", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	float ArmLengthDefault;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming|Camera", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	float ArmLengthAiming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming|Camera", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	float AimingSpeed;
	
	UFUNCTION()
	void OnAimingUpdate(float Value);

	#pragma endregion

	#pragma region ATTACKING_PRIVATE

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attacking", meta = (AllowPrivateAccess = true))
	TMap<EWeaponType, FAttackDescription> AttackAnimations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attacking", meta = (AllowPrivateAccess = true))
	TMap<EWeaponType, UAnimMontage*> ReloadAnimations;

	TMap<EWeaponType, TPair<FTimerHandle, bool>> RateOfFireDelays;

	UFUNCTION()
	void EnableDelayedShooting();

	UPROPERTY()
	EWeaponType RateOfFireToHandle;

	#pragma endregion

	#pragma region PUNCHING_PRIVATE
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punching", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> OnPunchHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punching", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	float FistDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punching", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	float PunchAimDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punching", meta = (AllowPrivateAccess = true))
	FName RightFistCollisionName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punching", meta = (AllowPrivateAccess = true))
	FName LeftFistCollisionName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punching", meta = (AllowPrivateAccess = true))
	int32 RightFistIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punching", meta = (AllowPrivateAccess = true))
	int32 LeftFistIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Punching", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Seconds"))
	float PunchCooldown;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UCooldown> RightFistCooldown;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UCooldown> LeftFistCooldown;

	UFUNCTION()
	void OnPunch(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	#pragma endregion
};
