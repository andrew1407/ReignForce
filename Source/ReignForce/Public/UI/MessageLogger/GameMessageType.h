#pragma once

#include "CoreMinimal.h"
#include "GameMessageType.generated.h"

UENUM(BlueprintType)
enum class EGameMessageType : uint8
{
    Ordinary UMETA(DisplayName="Ordinary"),
    Success UMETA(DisplayName="Success"),
    Failed UMETA(DisplayName="Failed"),
};
