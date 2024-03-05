#pragma once

UENUM(BlueprintType)
enum class EModalFocusMode : uint8
{
    None UMETA(DisplayName="None"),
    FocusOnAccept UMETA(DisplayName="FocusOnAccept"),
    FocusOnCancel UMETA(DisplayName="FocusOnCancel"),
};
