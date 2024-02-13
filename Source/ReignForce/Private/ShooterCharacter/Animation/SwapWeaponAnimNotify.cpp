// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter/Animation/SwapWeaponAnimNotify.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"

#include "ShooterCharacter/ShooterCharacter.h"


void USwapWeaponAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    auto Character = Cast<AShooterCharacter>(MeshComp->GetOwner());
    if (!IsValid(Character)) return;
    Character->SwapReservedWeapon();
}
