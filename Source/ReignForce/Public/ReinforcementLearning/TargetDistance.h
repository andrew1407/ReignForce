#pragma once

#include "CoreMinimal.h"
#include "TargetDistance.generated.h"

UENUM(BlueprintType)
enum class ETargetDistance : uint8
{
    None UMETA(DisplayName="None"),
    InAcceptedBounds UMETA(DisplayName="InAcceptedBounds"),
    Near UMETA(DisplayName="Near"),
    Far UMETA(DisplayName="Far"),
};
