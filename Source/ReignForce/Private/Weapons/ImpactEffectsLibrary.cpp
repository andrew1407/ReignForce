// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ImpactEffectsLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/DecalComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Pawn.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"

#include "Settings/ReignForceSettings.h"
#include "Weapons/ImpactEffectsDataAsset.h"


namespace
{
    // Small marks are kept visible from the shooting distance
    constexpr float DECAL_FADE_SCREEN_SIZE = .001f;
}

bool UImpactEffectsLibrary::SpawnSurfaceImpact(const UObject* WorldContextObject, const FHitResult& Hit, EWeaponType WeaponType, FVector MarkDirection)
{
    UImpactEffectsDataAsset* ImpactEffects = GetImpactEffects();
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (!(IsValid(ImpactEffects) && IsValid(World) && Hit.bBlockingHit)) return false;
    // Characters have own (blood) hit effects
    if (Cast<APawn>(Hit.GetActor())) return false;

    const EPhysicalSurface Surface = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
    bool bSurfaceEffect = ImpactEffects->Surfaces.Contains(Surface);
    const FSurfaceImpactEffect& Effect = ImpactEffects->GetEffect(Surface);
    const FVector Normal = Hit.ImpactNormal.GetSafeNormal();

    if (IsValid(Effect.Particles)) UGameplayStatics::SpawnEmitterAtLocation(
        World,
        Effect.Particles,
        Hit.ImpactPoint,
        Normal.Rotation(), // Debris bounce off along the surface normal
        Effect.ParticlesScale,
        true,
        EPSCPoolMethod::AutoRelease
    );

    if (IsValid(Effect.Sound))
        UGameplayStatics::SpawnSoundAtLocation(World, Effect.Sound, Hit.ImpactPoint, Normal.Rotation(), Effect.SoundVolume);

    const FImpactMark* Mark = Effect.Marks.Find(WeaponType);
    UPrimitiveComponent* HitComponent = Hit.GetComponent();
    if (!(Mark && IsValid(Mark->Decal) && IsValid(HitComponent))) return bSurfaceEffect;

    // Decal projects along its X axis into the surface, Y is the mark stretching direction
    FVector Along = FVector::VectorPlaneProject(MarkDirection, Normal);
    if (Along.IsNearlyZero()) Along = FVector::VectorPlaneProject(FMath::VRand(), Normal);
    const FRotator Rotation = FRotationMatrix::MakeFromXY(-Normal, Along).Rotator();
    const float SizeScale = 1 + FMath::FRandRange(-Mark->SizeRandomDeviation, Mark->SizeRandomDeviation);

    UDecalComponent* Decal = UGameplayStatics::SpawnDecalAttached(
        Mark->Decal,
        Mark->Size * FVector(1, SizeScale, SizeScale),
        HitComponent,
        NAME_None,
        Hit.ImpactPoint,
        Rotation,
        EAttachLocation::KeepWorldPosition
    );
    if (!IsValid(Decal)) return bSurfaceEffect;
    Decal->SetFadeScreenSize(DECAL_FADE_SCREEN_SIZE);
    constexpr bool bDestroyOwnerAfterFade = false;
    Decal->SetFadeOut(FMath::Max(0.f, Mark->LifeSpan - Mark->FadeOutDuration), Mark->FadeOutDuration, bDestroyOwnerAfterFade);
    return bSurfaceEffect;
}

UImpactEffectsDataAsset* UImpactEffectsLibrary::GetImpactEffects()
{
    const UReignForceSettings* Settings = UReignForceSettings::Get();
    return Settings ? Settings->ImpactEffects.LoadSynchronous() : nullptr;
}
