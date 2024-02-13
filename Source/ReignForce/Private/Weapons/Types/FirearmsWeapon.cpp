// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Types/FirearmsWeapon.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Controller.h"
#include "Perception/AISense_Damage.h"

#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"

#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"


AFirearmsWeapon::AFirearmsWeapon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    RateOfFire = .05f;
    Distance = 5000;
    AmmoCount = 0;
    MaxAmmoCount = 1;
    Recoil = FVector2D::ZeroVector;

    ShootingEffectScale = FVector(1);
    HitEffectScale = FVector(1);
    HitTargetEffectScale = FVector(1);

    FireSoundVolume = 1;
    EmptyGunShotSoundVolume = 1;
    HitSoundVolume = 1;
    HitTargetSoundVolume = 1;
}

FVector AFirearmsWeapon::GetAimingPosition_Implementation() const
{
    if (!IsValid(SkeletalMeshComponent)) return GetActorLocation();
	const FTransform SocketTransform = SkeletalMeshComponent->GetSocketTransform(MuzzlefashSocket, ERelativeTransformSpace::RTS_World);
    return SocketTransform.GetLocation();
}

bool AFirearmsWeapon::Fire(const FVector& Direction, const FRotator& Rotation, const FProjectileSpread& ProjectileSpread)
{
	if (!IsValid(SkeletalMeshComponent)) return false;
	const FVector SocketLocation = GetAimingPosition();
    if (GetIsAmmoEmpty())
    {
        PlayEmptyGunShotSound(SocketLocation);
        return false;
    }
    FHitResult HitResult; // This will store information about the hit object
    bool bHit = Aim(HitResult, AddDirectionSpread(Direction, ProjectileSpread));
    SpawnShootEffect(MuzzlefashSocket, SocketLocation, Rotation);
    --AmmoCount;
    PlayFireSound(SocketLocation);
    AActor* HitActor = HitResult.GetActor();
    const FName NoiseTag = *FString::Printf(TEXT("Firearms.%s"), *GetNameSafe(HitActor));
    if (Loudness > 0)
    {
        auto WeaponUser = Cast<APawn>(GetAttachParentActor());
        MakeNoise(Loudness, WeaponUser, SocketLocation, LoundnessRange, NoiseTag);
    }
	if (!bHit) return true;
    if (CheckAimTarget(HitActor))
    {
        SpawnHitTargetEffect(HitResult);
        PlayHitTargetSound(HitResult);
    }
    else
    {
        SpawnHitEffect(HitResult);
        PlayHitSound(HitResult);
    }
    const float DamageTaken = HitActor->TakeDamage(Damage, FDamageEvent(), nullptr, this);
    if (DamageTaken > 0) OnDamageCaused.Broadcast(DamageTaken, GetType(), HitActor, 0);
    UAISense_Damage::ReportDamageEvent(GetWorld(), HitActor, this, Damage, HitResult.ImpactPoint, HitResult.Location, NoiseTag);
    return true;
}

int32 AFirearmsWeapon::RefillAmmo(int32 AmmoToRefill, bool bSaveRest)
{
    if (GetIsAmmoFull()) return AmmoToRefill;
    if (!bSaveRest)
    {
        AmmoCount = FMath::Min(MaxAmmoCount, AmmoToRefill);
        return AmmoToRefill - MaxAmmoCount;
    }
    int32 ToRefillActual = FMath::Min(GetFreeSlots(), AmmoToRefill);
    AmmoCount += ToRefillActual;
    return AmmoToRefill - ToRefillActual;
}

UParticleSystemComponent* AFirearmsWeapon::SpawnShootEffect(const FName& Socket, const FVector& Location, const FRotator& Rotation) const
{
    return !IsValid(ShootingEffect) ? nullptr : UGameplayStatics::SpawnEmitterAttached(
		ShootingEffect,
		SkeletalMeshComponent,
		Socket, // The name of the socket on the parent component to attach to.
		Location,
		Rotation,
		ShootingEffectScale,
		EAttachLocation::KeepWorldPosition, // You can change the attachment rule as needed.
		true // Whether to auto destroy the emitter when the particle system is finished playing.
	);
}

UParticleSystemComponent* AFirearmsWeapon::SpawnHitEffect(const FHitResult& HitResult) const
{
    return !IsValid(HitEffect) ? nullptr : UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        HitEffect,
        HitResult.ImpactPoint,
        FQuat::FindBetween(FVector::RightVector, HitResult.Normal).Rotator(),
        HitEffectScale,
        true,
        EPSCPoolMethod::AutoRelease
    );
}

UParticleSystemComponent* AFirearmsWeapon::SpawnHitTargetEffect(const FHitResult& HitResult) const
{
    return !IsValid(HitTargetEffect) ? nullptr : UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        HitTargetEffect,
        HitResult.ImpactPoint,
        FQuat::FindBetween(FVector::RightVector, HitResult.Normal).Rotator(),
		HitTargetEffectScale,
        true,
        EPSCPoolMethod::AutoRelease
    );
}

UAudioComponent* AFirearmsWeapon::PlayFireSound(const FVector& Location) const
{
    return !IsValid(FireSound) ? nullptr : UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        FireSound,
        Location,
        FRotator::ZeroRotator,
        FireSoundVolume
    );
}

UAudioComponent* AFirearmsWeapon::PlayEmptyGunShotSound(const FVector& Location) const
{
    return !IsValid(EmptyGunShotSound) ? nullptr : UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        EmptyGunShotSound,
        Location,
        FRotator::ZeroRotator,
        EmptyGunShotSoundVolume
    );
}

UAudioComponent* AFirearmsWeapon::PlayHitSound(const FHitResult& HitResult) const
{
    return !IsValid(HitSound) ? nullptr : UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        HitSound,
        HitResult.ImpactPoint,
        FQuat::FindBetween(FVector::RightVector, HitResult.Normal).Rotator(),
        HitSoundVolume
    );
}

UAudioComponent* AFirearmsWeapon::PlayHitTargetSound(const FHitResult& HitResult) const
{
    return !IsValid(HitTargetSound) ? nullptr : UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        HitTargetSound,
        HitResult.ImpactPoint,
        FQuat::FindBetween(FVector::RightVector, HitResult.Normal).Rotator(),
        HitTargetSoundVolume
    );
}

FVector AFirearmsWeapon::AddDirectionSpread(const FVector& Direction, const FProjectileSpread& ProjectileSpread)
{
    if (ProjectileSpread.IsZero()) return Direction;
    if (ProjectileSpread.SpreadChacne < FMath::FRand()) return Direction;
    auto [ HorizontalConeHalfAngleRad, VerticalConeHalfAngleRad ] = ProjectileSpread.ToRadianPair();
    return FMath::VRandCone(Direction, HorizontalConeHalfAngleRad, VerticalConeHalfAngleRad); 
}
