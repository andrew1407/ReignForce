// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter/Animation/PlayAttackGruntAnimNotify.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"

#include "ShooterCharacter/ShooterCharacter.h"


void UPlayAttackGruntAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    auto Character = Cast<AShooterCharacter>(MeshComp->GetOwner());
    if (!IsValid(Character)) return;
    Character->GruntOnAttack();
}
