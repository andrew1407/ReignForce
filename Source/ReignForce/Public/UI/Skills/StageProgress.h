#pragma once

#include "CoreMinimal.h"
#include "StageProgress.generated.h"

UENUM(BlueprintType)
enum class EStageProgress: uint8
{
    None UMETA(DisplayName="None"),
    Locked UMETA(DisplayName="Locked"),
    InProgress UMETA(DisplayName="InProgress"),
    Completed UMETA(DisplayName="Completed"),
};
