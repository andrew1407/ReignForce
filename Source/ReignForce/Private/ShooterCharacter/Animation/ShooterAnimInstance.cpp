// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter/Animation/ShooterAnimInstance.h"
#include "GameFramework/PawnMovementComponent.h"

#include "ShooterCharacter/ShooterCharacter.h"
#include "Weapons/Components/WeaponSlotsSystem.h"
#include "Weapons/Components/CombatSystem.h"
#include "Weapons/ShooterState.h"


namespace
{
    constexpr float MIN_MOVE_DEVIATION = .05f;
    constexpr float START_MOVE_SPEED = 10;
}

UShooterAnimInstance::UShooterAnimInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    Speed = 0;
    SpeedXY = 0;
    Pitch = 0;

    bAiming = false;
    bRightMovementAiming = false;
    bFalling = false;
    bShouldMove = false;
    bFirearmsWeapon = false;
    bWeaponFreeUsage = false;
    
    WeaponType = EWeaponType::Unarmed;
    ShooterState = EShooterState::Idle;
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    Character = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!Character.IsValid()) return;
    UpdateCharacterParams();
    UpdateCombatParams();
    UpdateMovementParams();
    UpdateSlotParams();
}

void UShooterAnimInstance::UpdateCharacterParams()
{
    Velocity = Character->GetVelocity();
    Speed = Velocity.Length();
    SpeedXY = FVector2D(Velocity.X, Velocity.Y).Length();
    MovementDirection = Character->GetMovementDirection();
    Pitch = Character->GetAimOffsetPitch();
}

void UShooterAnimInstance::UpdateCombatParams()
{
    UCombatSystem* CombatSystem = Character->GetCombatSystem();
    if (!IsValid(CombatSystem)) return;

    bAiming = CombatSystem->GetAimingState();
    ShooterState = CombatSystem->ShooterState;
    bRightMovementAiming = bAiming && (FMath::Abs(MovementDirection.X) < MIN_MOVE_DEVIATION) && (FMath::Abs(MovementDirection.Y) > MIN_MOVE_DEVIATION);
}

void UShooterAnimInstance::UpdateMovementParams()
{
    UPawnMovementComponent* MovementComponent = Character->GetMovementComponent();
    if (!IsValid(MovementComponent)) return;

    bFalling = MovementComponent->IsFalling();
    bool bHasAcceleration = MovementComponent->GetLastInputVector() != FVector::ZeroVector;
    bShouldMove = bHasAcceleration && (Speed > START_MOVE_SPEED);
}

void UShooterAnimInstance::UpdateSlotParams()
{
    UWeaponSlotsSystem* SlotsSystem = Character->GetWeaponSlotsSystem();
    if (!IsValid(SlotsSystem)) return;

    WeaponType = SlotsSystem->GetActiveWeaponSlot();
    bFirearmsWeapon = SlotsSystem->IsFirearmsActive();

    bWeaponFreeUsage = ShooterState != EShooterState::Idle;
    if (ShooterState == EShooterState::Attacking)
        bWeaponFreeUsage = bWeaponFreeUsage && !bFirearmsWeapon;
}
