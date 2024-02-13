#pragma once

#include "PointsAccumulator.generated.h"


USTRUCT(BlueprintType)
struct FPointsAccumulator
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	float Speed = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	float Threshold = 1;

    UPROPERTY(BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	float CurrentValue = 0;

    FORCEINLINE bool ThresholdReached() const { return CurrentValue >= Threshold; }
    FORCEINLINE void AccumulateCurrent(float Delta) { CurrentValue += Speed * Delta; }
    FORCEINLINE void AccumulateCurrent(float Delta, float SpeedValue) { CurrentValue += SpeedValue * Delta; }
    float DropAccumulated()
    {
        const float ToDrop = CurrentValue;
        CurrentValue = 0;
        return ToDrop;
    }
};