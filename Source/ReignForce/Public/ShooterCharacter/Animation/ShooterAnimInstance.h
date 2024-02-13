// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Weapons/WeaponType.h"
#include "Weapons/ShooterState.h"
#include "ShooterAnimInstance.generated.h"


class AShooterCharacter;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UShooterAnimInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Refs", meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<AShooterCharacter> Character;

	#pragma region LOCOMOTION

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion", meta = (AllowPrivateAccess = true))
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion|Speed", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	float Speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion|Speed", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	float SpeedXY;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion", meta = (AllowPrivateAccess = true))
	FVector2D MovementDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion", meta = (AllowPrivateAccess = true))
	bool bRightMovementAiming;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion|Jump", meta = (AllowPrivateAccess = true))
	bool bFalling;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion|Jump", meta = (AllowPrivateAccess = true))
	bool bShouldMove;

	#pragma endregion

	#pragma region WEAPON

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Offset", meta = (AllowPrivateAccess = true))
	float Pitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Aiming", meta = (AllowPrivateAccess = true))
	bool bAiming;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = true))
	EWeaponType WeaponType;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = true))
	bool bFirearmsWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = true))
	bool bWeaponFreeUsage;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = true))
	EShooterState ShooterState;

	#pragma endregion

	void UpdateCharacterParams();
	void UpdateCombatParams();
	void UpdateMovementParams();
	void UpdateSlotParams();
};
