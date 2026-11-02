#pragma once

#include "CoreMinimal.h"
#include "WeaponType.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    NONE UMETA(DisplayName="None"),
    UNARMED UMETA(DisplayName="Unarmed"),
    RIFLE UMETA(DisplayName="Rifle"),
    PISTOL UMETA(DisplayName="Pistol"),
    MELEE UMETA(DisplayName="Melee"),
};
