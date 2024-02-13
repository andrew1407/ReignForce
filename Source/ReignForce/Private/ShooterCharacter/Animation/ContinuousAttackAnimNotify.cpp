// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter/Animation/ContinuousAttackAnimNotify.h"

#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"

#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"

#include "Weapons/WeaponType.h"
#include "Weapons/ShooterState.h"

#include "Weapons/Components/WeaponSlotsSystem.h"
#include "Weapons/Components/CombatSystem.h"


void UContinuousAttackAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    auto Character = Cast<AShooterCharacter>(MeshComp->GetOwner());
    if (!IsValid(Character) || Character->IsDead()) return;
    UWeaponSlotsSystem* SlotSystem = Character->GetWeaponSlotsSystem();
    if (!IsValid(SlotSystem)) return;
    UShooterSkillsSystem* SkillsSystem = Character->GetSkillsSystem();
    if (!IsValid(SkillsSystem)) return;
    UCombatSystem* CombatSystem = Character->GetCombatSystem();
    if (!IsValid(CombatSystem)) return;

    EWeaponType WeaponType = SlotSystem->GetActiveWeaponSlot();
	bool bContinuousInput = SkillsSystem->GetContituousAttackOrDefault(WeaponType);
    bool bAttackFinished = bContinuousInput && Character->GetShouldAttackState();
    if (!bAttackFinished)
    {
        CombatSystem->ShooterState = EShooterState::Idle;
        return;
    }
	const FWeaponSkillDescription WeaponInfo = SkillsSystem->GetWeaponAttackParamsOrDefault(WeaponType);
    CombatSystem->AttackWith(WeaponType, WeaponInfo.AnimationIndex, WeaponInfo.AnimationRate);
}
