#pragma once

#include "CoreMinimal.h"
#include "RoundState.generated.h"

UENUM(BlueprintType)
enum class ERoundState : uint8
{
    None UMETA(DisplayName="None"),
    Starting UMETA(DisplayName="Starting"),
    Going UMETA(DisplayName="Going"),
};
