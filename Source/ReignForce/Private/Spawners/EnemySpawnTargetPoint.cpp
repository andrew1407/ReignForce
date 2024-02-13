// Fill out your copyright notice in the Description page of Project Settings.


#include "Spawners/EnemySpawnTargetPoint.h"
#include "Math/UnrealMathUtility.h"
#include "NavigationSystem.h"


AEnemySpawnTargetPoint::AEnemySpawnTargetPoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bActiveToSpawn = true;
    Radius = 50;
    SpawnOncePerUsageCount = 1;
}

FVector AEnemySpawnTargetPoint::GetRandomCirclePoint(bool bReachableOnly) const
{
    return bReachableOnly ? GetRandomReachableCirclePoint() : GetRandomCircleAnyPoint();
}

FVector AEnemySpawnTargetPoint::GetRandomCircleAnyPoint() const
{
    const float RandomRadius = FMath::FRandRange(.0f, Radius);

    // Generate a random angle in radians
    const float RandomAngle = FMath::FRandRange(.0f, 2 * PI);

    // Calculate the coordinates in polar form
    const float X = RandomRadius * FMath::Cos(RandomAngle);
    const float Y = RandomRadius * FMath::Sin(RandomAngle);

    // Translate the polar coordinates to Cartesian coordinates
    const FVector RandomPoint = GetActorLocation() + FVector(X, Y, 0);
    return RandomPoint;
}

FVector AEnemySpawnTargetPoint::GetRandomReachableCirclePoint() const
{
    const FVector Origin = GetActorLocation();
    const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!IsValid(NavSystem)) return Origin;
    FNavLocation NavLocation;
    bool bLocationFound = NavSystem->GetRandomReachablePointInRadius(Origin, Radius, NavLocation);
    return bLocationFound ? NavLocation.Location : Origin;
}
