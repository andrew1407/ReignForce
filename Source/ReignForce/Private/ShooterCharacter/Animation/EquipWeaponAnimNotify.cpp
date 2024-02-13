// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter/Animation/EquipWeaponAnimNotify.h"

#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"
#include "ShooterCharacter/ShooterCharacter.h"


UEquipWeaponAnimNotify::UEquipWeaponAnimNotify(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bEquipAction = true;
}

void UEquipWeaponAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    auto Character = Cast<AShooterCharacter>(MeshComp->GetOwner());
    if (!IsValid(Character)) return;
    Character->ApplyAttachWeaponAction(bEquipAction);
}
