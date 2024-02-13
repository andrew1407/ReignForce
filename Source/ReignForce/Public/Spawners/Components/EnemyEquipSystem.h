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

	APlayerCharacter* GetPlayerCharacter() const;
	bool CheckSlotAvailability(UShooterSkillsSystem* SkillsSystem, EWeaponType Slot) const;
};
