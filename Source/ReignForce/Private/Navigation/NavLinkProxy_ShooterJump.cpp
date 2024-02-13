// Fill out your copyright notice in the Description page of Project Settings.


#include "Navigation/NavLinkProxy_ShooterJump.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"


namespace
{
    FVector CalulateJumpVelocity(const FVector& Start, const FVector& Destination, float Duration, float DirectionZ = -.5f, float GravitySpeed = 982)
    {
        FVector Velocity = Destination - Start;
        float CurrentValueZ = FMath::Square(Duration) * DirectionZ * GravitySpeed;
        Velocity.Z = Destination.Z - CurrentValueZ;
        return Velocity / Duration;
    }
}

ANavLinkProxy_ShooterJump::ANavLinkProxy_ShooterJump(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bSmartLinkIsRelevant = true;

    bActivateJumpAction = true;
    JumpDuration = 1;
    JumpActionProbability = 1;
}

void ANavLinkProxy_ShooterJump::BeginPlay()
{
    Super::BeginPlay();

    OnSmartLinkReached.AddDynamic(this, &ANavLinkProxy_ShooterJump::HandleSmartLinkReached);
}

void ANavLinkProxy_ShooterJump::HandleSmartLinkReached(AActor* Agent, const FVector& Destination)
{
    bool bShouldJump = bActivateJumpAction && JumpActionProbability >= FMath::FRand();
    if (!bShouldJump) return;

    auto Character = Cast<ACharacter>(Agent);
    if (!IsValid(Character)) return;

    const FVector JumpVelocity = CalulateJumpVelocity(Character->GetActorLocation(), Destination, JumpDuration);
    constexpr bool bXYOverride = true;
    constexpr bool bZOverride = true;
    Character->LaunchCharacter(JumpVelocity, bXYOverride, bZOverride);
    Character->Jump();
}
