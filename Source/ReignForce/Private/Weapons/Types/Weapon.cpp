// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Types/Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Actor.h"


// Sets default values
AWeapon::AWeapon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(GET_MEMBER_NAME_CHECKED(AWeapon, SkeletalMeshComponent));
	SkeletalMeshComponent->SetupAttachment(RootComponent);

    Loudness = 1;
	LoundnessRange = 0;

	Damage = 0;
	Distance = 0;
	Type = EWeaponType::None;
}

bool AWeapon::Aim(FHitResult& HitResult, const FVector& Direction) const
{
	const FVector AimLocation = GetAimingPosition();
	const FVector End = AimLocation + Direction * Distance;
    FCollisionQueryParams TraceParams(FName(TEXT("LineTrace")), false, this); // Additional parameters for the trace
	// TraceParams.AddIgnoredActor(this);
    AActor* WeaponUser = GetAttachParentActor();
    if (IsValid(WeaponUser)) TraceParams.AddIgnoredActor(WeaponUser);
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, AimLocation, End, ECollisionChannel::ECC_GameTraceChannel1, TraceParams);
	return bHit;
}

bool AWeapon::CheckAimTarget(AActor* Target) const
{
	for (const auto& AimClass : AimsToHit)
		if (Target->IsA(AimClass)) return true;
	return false;
}
