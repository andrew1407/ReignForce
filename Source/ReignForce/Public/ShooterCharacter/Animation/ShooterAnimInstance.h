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

	/**
	 * The jog blend space keeps the backward hemisphere (own idle, backward jog & strafes) shifted by it along the forward axis,
	 * apart from the forward one, so the triangulation never mixes strafes of both hemispheres
	 */
	static constexpr float BACKWARD_STRAFES_OFFSET = 1.f;

	/**
	 * Maps the movement direction circle onto the jog blend space diamond (idle center + cardinal jogs, foot sync markers aligned):
	 * keeps the movement magnitude as the jog weight, so a full speed diagonal (.7, .7) lands on the diamond edge (.5, .5)
	 * blending two jogs fully instead of leaking into idle, and a half speed one blends half idle like cardinals do.
	 *
	 * Each hemisphere has its own strafes: a strafe leads with the leg (in front) that matches the forward or the backward jog,
	 * otherwise the diagonal blend crosses the legs through each other (forward left & backward right used to break).
	 * The backward hemisphere is shifted onto its own diamond (centered at -BACKWARD_STRAFES_OFFSET), the forward one is clamped to its side.
	 */
	static FVector2D ToBlendSpaceDirection(const FVector2D& Direction, bool bBackwardStrafes = false);

	/** Picks the strafes hemisphere for a movement direction with a hysteresis, so a pure strafe doesn't flicker between both */
	static bool ShouldUseBackwardStrafes(const FVector2D& Direction, bool bWasBackward);

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = true))
	bool bBackwardStrafes;

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
