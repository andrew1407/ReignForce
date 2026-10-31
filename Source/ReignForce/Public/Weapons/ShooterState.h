#pragma once

#include "CoreMinimal.h"
#include "ShooterState.generated.h"

UENUM(BlueprintType)
enum class EShooterState : uint8
{
    Idle UMETA(DisplayName="Idle"),
    Attacking UMETA(DisplayName="Attacking"),
    SwitchingWeapon UMETA(DisplayName="SwitchingWeapon"),
    Reloading UMETA(DisplayName="Reloading"),
    Dead UMETA(DisplayName="Dead"),
};
