#pragma once

UENUM(BlueprintType)
enum class EPointsModifierType : uint8
{
    None UMETA(DisplayName="None"),
    DefaultInitialization UMETA(DisplayName="DefaultInitialization"),
    Damage UMETA(DisplayName="Damage"),
    Regeneration UMETA(DisplayName="Regeneration"),
    SkillUpgrade UMETA(DisplayName="SkillUpgrade"),
};