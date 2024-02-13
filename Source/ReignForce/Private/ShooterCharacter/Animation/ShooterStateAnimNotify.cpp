// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter/Animation/ShooterStateAnimNotify.h"

#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"

#include "Weapons/Components/CombatSystem.h"
#include "ShooterCharacter/ShooterCharacter.h"


UShooterStateAnimNotify::UShooterStateAnimNotify(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    ShooterState = EShooterState::Idle;
}

void UShooterStateAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    auto Character = Cast<AShooterCharacter>(MeshComp->GetOwner());
    if (!IsValid(Character) || Character->IsDead()) return;
    UCombatSystem* CombatSystem = Character->GetCombatSystem();
    if (!IsValid(CombatSystem)) return;

    CombatSystem->ShooterState = ShooterState;
}
