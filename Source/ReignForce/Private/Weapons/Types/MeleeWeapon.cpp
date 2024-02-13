// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Types/MeleeWeapon.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Perception/AISense_Damage.h"

#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"

#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

#include "Utils/Cooldown.h"


AMeleeWeapon::AMeleeWeapon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    DamageCollision = CreateDefaultSubobject<UBoxComponent>(GET_MEMBER_NAME_CHECKED(AMeleeWeapon, DamageCollision));
    if (IsValid(SkeletalMeshComponent)) DamageCollision->SetupAttachment(SkeletalMeshComponent);

    Cooldown = CreateDefaultSubobject<UCooldown>(GET_MEMBER_NAME_CHECKED(AMeleeWeapon, Cooldown));

    CooldownDuration = 0;
    OnHitSoundVolume = 1;
    HitTargetEffectScale = FVector(1);
    HitChance = 1;
}

void AMeleeWeapon::BeginPlay()
{
    Super::BeginPlay();

    DisableCollision();
}

FVector AMeleeWeapon::GetAimingPosition_Implementation() const
{
    const AActor* WeaponUser = GetAttachParentActor();
    return IsValid(WeaponUser) ? WeaponUser->GetActorLocation() : GetActorLocation();
}

void AMeleeWeapon::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

	if (HitChance < FMath::FRand()) return;
    if (CooldownDuration != 0 && IsValid(Cooldown))
    {
        if (Cooldown->HasCooldown(OtherActor)) return;
        Cooldown->Activate(OtherActor, CooldownDuration);
    }

    AActor* WeaponUser = GetAttachParentActor();
    if (IsValid(WeaponUser) && WeaponUser == OtherActor) return;
    const FName NoiseTag = *FString::Printf(TEXT("Melee.%s"), *GetNameSafe(OtherActor));
    if (CheckAimTarget(OtherActor))
    {
        SpawnHitTargetEffect();
        PlayOnHitSound();
        if (Loudness > 0) MakeNoise(Loudness, Cast<APawn>(WeaponUser), GetActorLocation(), LoundnessRange, NoiseTag);
    }
    const float DamageTaken = OtherActor->TakeDamage(Damage, FDamageEvent(), nullptr, this);
    if (DamageTaken > 0) OnDamageCaused.Broadcast(DamageTaken, GetType(), OtherActor, 0);
    UAISense_Damage::ReportDamageEvent(GetWorld(), OtherActor, this, Damage, OtherActor->GetActorLocation(), GetActorLocation(), NoiseTag);
}

void AMeleeWeapon::EnableCollision(int32 Index)
{
    if (IsValid(DamageCollision))
        DamageCollision->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
}

void AMeleeWeapon::DisableCollision(int32 Index)
{
    if (IsValid(DamageCollision))
       DamageCollision->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void AMeleeWeapon::SetCollisionState(bool bState, int32 Index)
{
    if (bState) EnableCollision(Index);
    else DisableCollision(Index);
}

bool AMeleeWeapon::IsDamageCollisionEnabled(int32 Index) const
{
    if (!IsValid(DamageCollision)) return false;
    return DamageCollision->GetCollisionEnabled() == ECollisionEnabled::Type::QueryAndPhysics;
}

UAudioComponent* AMeleeWeapon::PlayOnHitSound() const
{
    return !IsValid(OnHitSound) ? nullptr : UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        OnHitSound,
        GetActorLocation(),
        GetActorRotation(),
        OnHitSoundVolume
    );
}

UParticleSystemComponent* AMeleeWeapon::SpawnHitTargetEffect() const
{
    return !IsValid(HitTargetEffect) ? nullptr : UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        HitTargetEffect,
        GetActorLocation(),
        GetActorRotation(),
        HitTargetEffectScale,
        true,
        EPSCPoolMethod::AutoRelease
    );
}
