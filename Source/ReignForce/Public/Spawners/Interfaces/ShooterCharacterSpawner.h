// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ShooterCharacterSpawner.generated.h"


class AShooterCharacter;
class ATargetPoint;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnShootersSpawned, bool, bSuccess);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnShooterCreatedDeferred, AShooterCharacter*, Shooter);

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UShooterCharacterSpawner : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class REIGNFORCE_API IShooterCharacterSpawner
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ShooterCharacterSpawner")
	TArray<AShooterCharacter*> GetSpawnedShooters();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ShooterCharacterSpawner")
	int32 GetSpawnedCount();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ShooterCharacterSpawner")
	bool HasSpawnPoints();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ShooterCharacterSpawner")
	int32 GetAliveSpawnedCount();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ShooterCharacterSpawner")
	AShooterCharacter* FindSpawnedByName(const FString& ShooterName);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ShooterCharacterSpawner")
	void CollectSpawnPoints();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ShooterCharacterSpawner")
	bool SpawnShooter(const FOnShooterCreatedDeferred& OnShooterCreatedDeferred, ATargetPoint* SpawnPoint = nullptr, TSubclassOf<AShooterCharacter> ToSpawn = nullptr, int32 Amount = 1);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ShooterCharacterSpawner")
	void SpawnShooterBunch(const FOnShooterCreatedDeferred& OnShooterCreatedDeferred, int32 Bunch = -1);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ShooterCharacterSpawner")
	void SpawnShooterBunchAsync(const FOnShooterCreatedDeferred& OnShooterCreatedDeferred, const FOnShootersSpawned& OnShootersSpawned, int32 Bunch = -1);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ShooterCharacterSpawner")
	void ClearSpawnedShooters();
};
