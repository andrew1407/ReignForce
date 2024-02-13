#pragma once

UENUM(BlueprintType)
enum class EPlayerInteractionMode : uint8
{
    None UMETA(DisplayName="None"),
    Pause UMETA(DisplayName="Pause"),
    NotPause UMETA(DisplayName="NotPause"),
    UIInput UMETA(DisplayName="UIInput"),
    UIInputAndPause UMETA(DisplayName="UIInputAndPause"),
    GameplayInput UMETA(DisplayName="GameplayInput"),
    GameplayInputAndNotPause UMETA(DisplayName="GameplayInputAndNotPause"),
};
