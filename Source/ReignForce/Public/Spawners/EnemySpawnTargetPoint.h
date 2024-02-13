// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "EnemySpawnTargetPoint.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API AEnemySpawnTargetPoint : public ATargetPoint
{
	GENERATED_BODY()

public:
	AEnemySpawnTargetPoint(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bActiveToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = 0, UIMin = 0))
	float Radius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = 0, UIMin = 0))
	int32 SpawnOncePerUsageCount;

	UFUNCTION(BlueprintPure)
	FVector GetRandomCirclePoint(bool bReachableOnly = true) const;

	UFUNCTION(BlueprintPure)
	FVector GetRandomCircleAnyPoint() const;

	UFUNCTION(BlueprintPure)
	FVector GetRandomReachableCirclePoint() const;
};
