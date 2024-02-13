// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter/Animation/AfterReloadStateAnimNotify.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"

#include "ShooterCharacter/ShooterCharacter.h"
#include "Weapons/Components/CombatSystem.h"


void UAfterReloadStateAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    auto Character = Cast<AShooterCharacter>(MeshComp->GetOwner());
    if (!IsValid(Character) || Character->IsDead()) return;
    UCombatSystem* CombatSystem = Character->GetCombatSystem();
    if (!IsValid(CombatSystem)) return;
    CombatSystem->ShooterState = Character->GetShouldAttackState() ? EShooterState::Attacking : EShooterState::Idle;
}
