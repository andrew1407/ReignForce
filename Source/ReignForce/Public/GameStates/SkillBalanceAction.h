#pragma once

UENUM(BlueprintType)
enum class ESkillBalanceAction : uint8
{
    None UMETA(DisplayName="None"),
    Initialize UMETA(DisplayName="Initialize"),
    Add UMETA(DisplayName="Add"),
    Take UMETA(DisplayName="Take"),
    Drop UMETA(DisplayName="Drop"),
};
