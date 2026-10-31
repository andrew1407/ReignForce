#pragma once

#include "CoreMinimal.h"
#include "TemperamentType.generated.h"

UENUM(BlueprintType)
enum class ETemperamentType : uint8
{
    None UMETA(DisplayName="None"),
    Agressive UMETA(DisplayName="Agressive"),
    Retreat UMETA(DisplayName="Retreat"),
    Moderate UMETA(DisplayName="Moderate"),
};
