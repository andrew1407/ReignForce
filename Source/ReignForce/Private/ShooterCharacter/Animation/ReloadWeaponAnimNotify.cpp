// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter/Animation/ReloadWeaponAnimNotify.h"

#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"

#include "Weapons/Components/WeaponSlotsSystem.h"
#include "Weapons/WeaponType.h"
#include "ShooterCharacter/ShooterCharacter.h"


void UReloadWeaponAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    auto Character = Cast<AShooterCharacter>(MeshComp->GetOwner());
    if (!IsValid(Character)) return;
    UWeaponSlotsSystem* SlotsSystem = Character->GetWeaponSlotsSystem();
    if (!IsValid(SlotsSystem)) return;

    Character->RefillAmmo(SlotsSystem->GetActiveWeaponSlot());
}
