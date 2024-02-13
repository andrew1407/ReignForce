#pragma once

UENUM(BlueprintType)
enum class ETeamID : uint8
{
    None UMETA(DisplayName="None"),
    Neutral UMETA(DisplayName="Neutral"),
    Player UMETA(DisplayName="Player"),
    Enemy UMETA(DisplayName="Enemy"),
};
