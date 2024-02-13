#pragma once

#include "ReinforcementLearning/TargetDistance.h"
#include "Containers/UnrealString.h"
#include "AIClientRequest.generated.h"


USTRUCT(BlueprintType)
struct FShooterMetrics
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
    int32 ShooterState = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsMoving = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
    float Health = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, ClampMax = 1, UIMax = 1))
    float HealthPercent = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsFirearmsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
    int32 ActiveWeapon = 0;
};

USTRUCT(BlueprintType)
struct FAIClientInput
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Reward = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsDone = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsTargetVisible = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
    int32 Command = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
    float TargetDistance = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
    int32 DistanceState = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FShooterMetrics SelfMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FShooterMetrics TargetMetrics;
};

USTRUCT(BlueprintType)
struct FAIClientRequest
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Event;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsInitial = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
    int32 Step = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FAIClientInput ClientInput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FAIClientInput PreviousClientInput;
};
