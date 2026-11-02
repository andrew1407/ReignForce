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
#include "Weapons/ImpactEffectsLibrary.h"


AMeleeWeapon::AMeleeWeapon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    DamageCollision = CreateDefaultSubobject<UBoxComponent>(GET_MEMBER_NAME_CHECKED(AMeleeWeapon, DamageCollision));
    if (IsValid(SkeletalMeshComponent)) DamageCollision->SetupAttachment(SkeletalMeshComponent);

    Cooldown = CreateDefaultSubobject<UCooldown>(GET_MEMBER_NAME_CHECKED(AMeleeWeapon, Cooldown));

    CooldownDuration = 0;
    OnHitSoundVolume = 1;
    HitTargetEffectScale = FVector(1);
    HitChance = 1;

    bMarkSurfaces = true;
    bSurfaceHitDuringSwing = false;
    LastSwingLocation = FVector::ZeroVector;
    // Ticks only while the damage collision is enabled (swing)
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
}

void AMeleeWeapon::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    TraceSwingSurfaceHit();
}

void AMeleeWeapon::TraceSwingSurfaceHit()
{
    if (!(bMarkSurfaces && IsValid(DamageCollision)) || bSurfaceHitDuringSwing) return;

    const FVector SwingLocation = DamageCollision->GetComponentLocation();
    const FVector SwingStart = LastSwingLocation;
    LastSwingLocation = SwingLocation;
    if (SwingStart.Equals(SwingLocation)) return;

    FCollisionQueryParams Params(TEXT("MeleeSurfaceTrace"), false, this);
    Params.bReturnPhysicalMaterial = true;
    Params.AddIgnoredActor(GetAttachParentActor());
    FCollisionObjectQueryParams Objects;
    Objects.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);
    Objects.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
    const FCollisionShape Blade = FCollisionShape::MakeBox(DamageCollision->GetScaledBoxExtent());

    FHitResult Hit;
    bool bHit = GetWorld()->SweepSingleByObjectType(Hit, SwingStart, SwingLocation, DamageCollision->GetComponentQuat(), Objects, Blade, Params);
    const AActor* HitActor = Hit.GetActor();
    if (!bHit || (IsValid(HitActor) && (HitActor->IsA<APawn>() || HitActor->IsA<AWeapon>()))) return;

    // A single mark per swing, the blade stretches it along the swing direction
    bSurfaceHitDuringSwing = true;
    UImpactEffectsLibrary::SpawnSurfaceImpact(this, Hit, GetType(), SwingLocation - SwingStart);
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
    if (!IsValid(DamageCollision)) return;
    DamageCollision->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);

    bSurfaceHitDuringSwing = false;
    LastSwingLocation = DamageCollision->GetComponentLocation();
    SetActorTickEnabled(bMarkSurfaces);
}

void AMeleeWeapon::DisableCollision(int32 Index)
{
    SetActorTickEnabled(false);
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
