// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter/Animation/ApplyMeleeAnimNotifyState.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"
#include "ShooterCharacter/ShooterCharacter.h"


UApplyMeleeAnimNotifyState::UApplyMeleeAnimNotifyState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    CollisionParam = 0;
}

void UApplyMeleeAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    SetCollisionState(MeshComp->GetOwner(), true);
}

void UApplyMeleeAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    SetCollisionState(MeshComp->GetOwner(), false);
}

void UApplyMeleeAnimNotifyState::SetCollisionState(AActor* Actor, bool bState)
{
    auto Character = Cast<AShooterCharacter>(Actor);
    if (!IsValid(Character)) return;
    Character->TryActivateMeleeCollision(bState, CollisionParam);
}
