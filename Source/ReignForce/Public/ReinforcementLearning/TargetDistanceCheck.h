#pragma once

#include "Containers/Set.h"
#include "ReinforcementLearning/TargetDistance.h"
#include "TargetDistanceCheck.generated.h"


USTRUCT(BlueprintType)
struct FTargetDistanceCheck
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSoleComparison = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bSoleComparison", EditConditionHides))
    ETargetDistance DistanceState = ETargetDistance::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bSoleComparison", EditConditionHides))
    TSet<ETargetDistance> DistanceStates;

    bool Check(ETargetDistance Distance) const
    {
        return bSoleComparison ? DistanceState == Distance : DistanceStates.Contains(Distance);
    }
};