// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Weapons/WeaponType.h"
#include "ImpactEffectsLibrary.generated.h"


class UImpactEffectsDataAsset;

/**
 * Spawns surface dependent hit effects: debris particles, sound & a mark (bullet hole, blade cut)
 */
UCLASS()
class REIGNFORCE_API UImpactEffectsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @param MarkDirection - mark stretching direction along the surface (blade swing), random when zero
	 * @return true if the surface has own effects
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapons|Impacts", meta = (WorldContext = "WorldContextObject"))
	static bool SpawnSurfaceImpact(const UObject* WorldContextObject, const FHitResult& Hit, EWeaponType WeaponType, FVector MarkDirection = FVector::ZeroVector);

	UFUNCTION(BlueprintPure, Category = "Weapons|Impacts")
	static UImpactEffectsDataAsset* GetImpactEffects();
};
