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

    // Movement direction cosine to the forward axis: enters the backward strafes below the first one, leaves above the second
    constexpr float BACKWARD_STRAFES_ENTER = -.15f;
    constexpr float BACKWARD_STRAFES_EXIT = -.02f;
}

FVector2D UShooterAnimInstance::ToBlendSpaceDirection(const FVector2D& Direction, bool bBackwardStrafes)
{
    const float AxesSum = FMath::Abs(Direction.X) + FMath::Abs(Direction.Y);
    if (AxesSum <= KINDA_SMALL_NUMBER) return FVector2D::ZeroVector;
    FVector2D Result = Direction * (FMath::Min(Direction.Size(), 1.f) / AxesSum);

    if (bBackwardStrafes) Result.X = FMath::Min(Result.X, 0.) - BACKWARD_STRAFES_OFFSET;
    else Result.X = FMath::Max(Result.X, 0.);
    return Result;
}

bool UShooterAnimInstance::ShouldUseBackwardStrafes(const FVector2D& Direction, bool bWasBackward)
{
    const double Size = Direction.Size();
    if (Size <= KINDA_SMALL_NUMBER) return bWasBackward;
    const double ForwardCos = Direction.X / Size;
    return ForwardCos < (bWasBackward ? BACKWARD_STRAFES_EXIT : BACKWARD_STRAFES_ENTER);
}

UShooterAnimInstance::UShooterAnimInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    Speed = 0;
    SpeedXY = 0;
    Pitch = 0;

    bAiming = false;
    bRightMovementAiming = false;
    bBackwardStrafes = false;
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
    UpdateMovementParams();
    UpdateCombatParams();
    UpdateSlotParams();
}

void UShooterAnimInstance::UpdateCharacterParams()
{
    Velocity = Character->GetVelocity();
    SpeedXY = FVector2D(Velocity.X, Velocity.Y).Length();
    const FVector2D Direction = Character->GetMovementDirection();
    bBackwardStrafes = ShouldUseBackwardStrafes(Direction, bBackwardStrafes);
    MovementDirection = ToBlendSpaceDirection(Direction, bBackwardStrafes);
    Pitch = Character->GetAimOffsetPitch();

    UPawnMovementComponent* MovementComponent = Character->GetMovementComponent();
    bFalling = IsValid(MovementComponent) && MovementComponent->IsFalling();

    // Ground locomotion speed: upper body holding poses switch to running (arms swinging) by it,
    // so it's dropped while in the air (jump / fall) not to keep running arms with legs in a jump pose
    Speed = bFalling ? 0 : Velocity.Length();
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

    bool bHasAcceleration = MovementComponent->GetLastInputVector() != FVector::ZeroVector;
    bShouldMove = bHasAcceleration && (SpeedXY > START_MOVE_SPEED);
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
