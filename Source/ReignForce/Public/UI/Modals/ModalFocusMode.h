#pragma once

#include "CoreMinimal.h"
#include "ModalFocusMode.generated.h"

UENUM(BlueprintType)
enum class EModalFocusMode : uint8
{
    None UMETA(DisplayName="None"),
    FocusOnAccept UMETA(DisplayName="FocusOnAccept"),
    FocusOnCancel UMETA(DisplayName="FocusOnCancel"),
};
