// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Weapons/WeaponType.h"
#include "ImpactEffectsDataAsset.generated.h"


class UParticleSystem;
class UMaterialInterface;
class USoundBase;

USTRUCT(BlueprintType)
struct FImpactMark
{
	GENERATED_USTRUCT_BODY()

	/** Deferred decal material left on the hit surface */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInterface> Decal;

	/** Decal projection box extents (X - depth, Y & Z - width & height) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Size = FVector(4, 6, 6);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	float SizeRandomDeviation = .25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, Units = "Seconds"))
	float LifeSpan = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, Units = "Seconds"))
	float FadeOutDuration = 3;
};

USTRUCT(BlueprintType)
struct FSurfaceImpactEffect
{
	GENERATED_USTRUCT_BODY()

	/** Debris, dust & sparks flying off the surface */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UParticleSystem> Particles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ParticlesScale = FVector(1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	float SoundVolume = 1;

	/** Marks per weapon kind (bullet hole, blade cut) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EWeaponType, FImpactMark> Marks;
};

/**
 * Weapon hits look per physical surface of the hit (non-target) objects
 */
UCLASS(BlueprintType)
class REIGNFORCE_API UImpactEffectsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impacts")
	TMap<TEnumAsByte<EPhysicalSurface>, FSurfaceImpactEffect> Surfaces;

	/** Used for surfaces without own effects */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impacts")
	FSurfaceImpactEffect Default;

	const FSurfaceImpactEffect& GetEffect(EPhysicalSurface Surface) const;
};
