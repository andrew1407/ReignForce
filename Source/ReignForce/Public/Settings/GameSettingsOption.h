#pragma once

#include "CoreMinimal.h"
#include "GameSettingsOption.generated.h"

/** Settings menu option rows in their menu order */
UENUM(BlueprintType)
enum class EGameSettingsOption : uint8
{
    WINDOW_MODE UMETA(DisplayName="Window mode"),
    RESOLUTION UMETA(DisplayName="Resolution"),
    QUALITY UMETA(DisplayName="Graphics quality"),
    VSYNC UMETA(DisplayName="VSync"),
    FRAME_RATE UMETA(DisplayName="Frame rate limit"),
    LOOK_SENSITIVITY UMETA(DisplayName="Camera sensitivity"),
    MASTER_VOLUME UMETA(DisplayName="Master volume"),
    MUSIC_VOLUME UMETA(DisplayName="Music volume"),
    EFFECTS_VOLUME UMETA(DisplayName="Game sounds volume"),
    INVERT_MOUSE_X UMETA(DisplayName="Invert mouse left / right"),
    INVERT_MOUSE_Y UMETA(DisplayName="Invert mouse up / down"),
    INVERT_GAMEPAD_X UMETA(DisplayName="Invert gamepad stick left / right"),
    INVERT_GAMEPAD_Y UMETA(DisplayName="Invert gamepad stick up / down"),
    COUNT UMETA(Hidden),
};
