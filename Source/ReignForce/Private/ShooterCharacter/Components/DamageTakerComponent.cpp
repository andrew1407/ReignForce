// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter/Components/DamageTakerComponent.h"
#include "Engine/DamageEvents.h"

#include "Weapons/Types/Weapon.h"


// Sets default values for this component's properties
UDamageTakerComponent::UDamageTakerComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MinFallDamage = 10;
	MaxFallDamage = 200;
	MinHeightFallSpeed = 870;
	MaxHeightFallSpeed = 2000;
}

EWeaponType UDamageTakerComponent::DefineDamage(AActor* Target, TSubclassOf<AActor> PunchCauser) const
{
	if (PunchCauser && Target->IsA(PunchCauser)) return EWeaponType::Unarmed;
	else if (auto Weapon = Cast<AWeapon>(Target)) return Weapon->GetType();
	return EWeaponType::None;
}

float UDamageTakerComponent::ApplyFallDamage(float& DamagePercentage, float Velocity, AActor* LandSurface)
{
	if (Velocity < MinHeightFallSpeed) return 0;
	float FallVelocity = FMath::Min(Velocity, MaxHeightFallSpeed);
	DamagePercentage = (FallVelocity - MinHeightFallSpeed) / (MaxHeightFallSpeed - MinHeightFallSpeed);
	const float FallDamage = DamagePercentage * MaxFallDamage + MinFallDamage;
	if (FMath::IsNearlyZero(FallDamage)) return 0;
	
	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor)) return 0;
	return OwnerActor->TakeDamage(FallDamage, FDamageEvent(), nullptr, LandSurface);
}
