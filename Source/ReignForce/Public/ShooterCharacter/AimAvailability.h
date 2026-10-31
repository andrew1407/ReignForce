#pragma once

#include "CoreMinimal.h"
#include "AimAvailability.generated.h"

UENUM(BlueprintType)
enum class EAimAvailability : uint8
{
    NoAim UMETA(DisplayName="NoAim"),
    AimPresent UMETA(DisplayName="AimPresent"),
    AimTraceAimingOnly UMETA(DisplayName="AimTraceAimingOnly"),
    AimTraceAlways UMETA(DisplayName="AimTraceAlways"),
};
