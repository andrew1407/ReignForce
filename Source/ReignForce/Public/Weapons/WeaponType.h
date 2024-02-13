#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    None UMETA(DisplayName="None"),
    Unarmed UMETA(DisplayName="Unarmed"),
    Rifle UMETA(DisplayName="Rifle"),
    Pistol UMETA(DisplayName="Pistol"),
    Melee UMETA(DisplayName="Melee"),
};
