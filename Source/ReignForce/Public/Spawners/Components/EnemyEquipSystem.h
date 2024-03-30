// Fill out your copy:right notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Spawners/Interfaces/ShooterCharacterEquipper.h"
#include "Weapons/WeaponType.h"
#include "ReinforcementLearning/TemperamentType.h"
#include "EnemyEquipSystem.generated.h"


class AShooterCharacter;
class APlayerCharacter;
class UShooterSkillsSystem;

USTRUCT(BlueprintType)
struct FWeaponAvailability
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, ClampMax = 1, UIMax = 1, Units = "Times"))
    float DefaultProbability = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, ClampMax = 1, UIMax = 1, Units = "Times"))
    float AlternativeProbability = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bApplyExtraCheck = false;
};

USTRUCT(BlueprintType)
struct FRankDeviation
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bApplyDeviation", EditConditionHides))
    int32 Left = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bApplyDeviation", EditConditionHides))
    int32 Right = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bApplyDeviation = true;

    int32 Apply(int32 Rank, int32 LowerBound = 0, int32 UpperBound = MAX_int32) const;
	int32 operator()(int32 Rank, int32 LowerBound = 0, int32 UpperBound = MAX_int32) const;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REIGNFORCE_API UEnemyEquipSystem : public UActorComponent, public IShooterCharacterEquipper
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemyEquipSystem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ProvideWithAvailableWeapons_Implementation(AShooterCharacter* ShooterCharacter) override;
	virtual void ProvideWithRanksProgression_Implementation(AShooterCharacter* ShooterCharacter) override;
	virtual void AdjustBehavior_Implementation(AShooterCharacter* ShooterCharacter) override;
	virtual void DistributeBehaviors_Implementation(const TArray<AShooterCharacter*>& ShooterCharacters) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = true))
	TMap<EWeaponType, FWeaponAvailability> WeaponEquipProbabilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Temperament", meta = (AllowPrivateAccess = true))
	TMap<ETemperamentType, float> TemperamentProbabilities;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Temperament", meta = (AllowPrivateAccess = true))
	TMap<ETemperamentType, float> TemperamentDistributions;

	#pragma region RANK_DEVIATIONS

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deviation", meta = (AllowPrivateAccess = true))
	FRankDeviation DefaultDeviation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deviation", meta = (AllowPrivateAccess = true))
	FRankDeviation WeaponReloadSpeedDeviation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deviation", meta = (AllowPrivateAccess = true))
	FRankDeviation WeaponSwapSpeedDeviation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deviation", meta = (AllowPrivateAccess = true))
	FRankDeviation AimingSpeedDeviation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deviation", meta = (AllowPrivateAccess = true))
	FRankDeviation MaxHealthDeviation;

	#pragma endregion

	APlayerCharacter* GetPlayerCharacter() const;
	bool CheckSlotAvailability(UShooterSkillsSystem* SkillsSystem, EWeaponType Slot) const;
};
