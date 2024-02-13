// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Spawners/Interfaces/ShooterCharacterSpawner.h"
#include "EnemySpawnerComponent.generated.h"


class AShooterCharacter;
class AEnemyCharacter;

class AEnemySpawnTargetPoint;
class ATargetPoint;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REIGNFORCE_API UEnemySpawnerComponent : public UActorComponent, public IShooterCharacterSpawner
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemySpawnerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	#pragma region STATE

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TArray<TSubclassOf<AEnemyCharacter>> EnemiesToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = 1, UIMin = 1))
	int32 SpawnBunch;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TArray<TObjectPtr<AEnemyCharacter>> SpawnedEnemies;
			
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	TArray<AEnemySpawnTargetPoint*> SpawnPoints;

	#pragma endregion

	#pragma region SPAWNER_IMPLEMENTATION

	FORCEINLINE virtual int32 GetSpawnedCount_Implementation() override { return SpawnedEnemies.Num(); }
	FORCEINLINE virtual bool HasSpawnPoints_Implementation() override { return !SpawnPoints.IsEmpty(); }

	virtual TArray<AShooterCharacter*> GetSpawnedShooters_Implementation() override;
	virtual int32 GetAliveSpawnedCount_Implementation() override;
	virtual AShooterCharacter* FindSpawnedByName_Implementation(const FString& ShooterName) override;

	virtual void CollectSpawnPoints_Implementation() override;

	virtual bool SpawnShooter_Implementation(const FOnShooterCreatedDeferred& OnShooterCreatedDeferred, ATargetPoint* SpawnPoint = nullptr,
											TSubclassOf<AShooterCharacter> ToSpawn = nullptr, int32 Amount = 1) override;

	virtual void SpawnShooterBunch_Implementation(const FOnShooterCreatedDeferred& OnShooterCreatedDeferred, int32 Bunch = -1) override;

	virtual void SpawnShooterBunchAsync_Implementation(const FOnShooterCreatedDeferred& OnShooterCreatedDeferred,
														const FOnShootersSpawned& OnShootersSpawned, int32 Bunch = -1) override;
	void ClearSpawnedShooters_Implementation();

	#pragma endregion

private:
	UFUNCTION()
	void SpawnPerTick(const FOnShooterCreatedDeferred& OnShooterCreatedDeferred, const FOnShootersSpawned& OnShootersSpawned, int32 Left);

	bool GetRandomEnemyToSpawn(TSubclassOf<AShooterCharacter>& Container) const;
	bool GetRandomSpawnPoint(ATargetPoint*& Container) const;
};
