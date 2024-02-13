#pragma once

#include "Containers/Map.h"
#include "Weapons/WeaponType.h"
#include "RanksProgression.generated.h"


USTRUCT(BlueprintType)
struct FRanksProgression
{
    GENERATED_USTRUCT_BODY()

    #pragma region INDICES

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 WeaponReload = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 WeaponSwap = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 Aiming = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 Health = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 AimAvailability = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 HealthRegenerationSpeed = 0;

    #pragma endregion

    #pragma region STATES

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bWeaponAutoReloadMode = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSaveAmmoMode = false;

    #pragma endregion

    #pragma region COLLECTIONS

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EWeaponType, int32> WeaponAttacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EWeaponType, int32> AmmoCapaticy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EWeaponType, bool> ContiniousAttackModes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EWeaponType, int32> WeaponRecoilModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EWeaponType, int32> MeleeAttackHitChances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EWeaponType, int32> FirearmsProjectileSpreads;

    #pragma endregion
};
