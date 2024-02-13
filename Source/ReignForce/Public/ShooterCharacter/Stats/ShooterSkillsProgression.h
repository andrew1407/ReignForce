// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Weapons/WeaponType.h"
#include "Weapons/ProjectileSpread.h"
#include "ShooterCharacter/AimAvailability.h"
#include "ShooterSkillsProgression.generated.h"


UENUM(BlueprintType)
enum class ESkillUnlockAbility : uint8
{
    AlwaysFalse UMETA(DisplayName="AlwaysFalse"),
    AlwaysTrue UMETA(DisplayName="AlwaysTrue"),
    FromFalseToTrue UMETA(DisplayName="FromFalseToTrue"),
    FromTrueToFalse UMETA(DisplayName="FromTrueToFalse"),
};

USTRUCT(BlueprintType)
struct FWeaponSkillDescription
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 AnimationIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, Units = "Times"))
	float AnimationRate = 1;
};

USTRUCT(BlueprintType)
struct FWeaponSkillDescriptionCollection
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWeaponSkillDescription> Collection;
};

USTRUCT(BlueprintType)
struct FAmmoCapacity
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	TArray<int32> MaxCount;
};

USTRUCT(BlueprintType)
struct FRecoilModifiersCollection
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector2D> Collection;
};

USTRUCT(BlueprintType)
struct FAttackHitChancesCollection
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, ClampMax = 1, UIMax = 1))
	TArray<float> Collection;
};

USTRUCT(BlueprintType)
struct FProjectileSpreadsCollection
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FProjectileSpread> Collection;
};

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UShooterSkillsProgression : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<EAimAvailability> AimAvailability;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Weapon", meta = (ClampMin = 0, UIMin = 0))
	TArray<float> WeaponSwappingSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Weapon", meta = (ClampMin = 0, UIMin = 0))
	TArray<float> AimingSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Weapon")
	TMap<EWeaponType, ESkillUnlockAbility> ContinuousAttacks;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Weapon")
	TMap<EWeaponType, FWeaponSkillDescriptionCollection> WeaponAttacks;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Firearms", meta = (ClampMin = 0, UIMin = 0))
	TArray<float> WeaponReloadSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Firearms")
	ESkillUnlockAbility FirearmsAutoReload;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Firearms")
	ESkillUnlockAbility AmmoSaveMode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Firearms")
	TMap<EWeaponType, FRecoilModifiersCollection> WeaponRecoilModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	TArray<int32> MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = 0, UIMin = 0))
	TArray<float> HealthRegenerationSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	TMap<EWeaponType, FAmmoCapacity> AmmoStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Accuracy")
    TMap<EWeaponType, FAttackHitChancesCollection> MeleeAttackHitChances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Accuracy")
    TMap<EWeaponType, FProjectileSpreadsCollection> FirearmsProjectileSpreads;
};
