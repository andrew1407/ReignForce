// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ShooterCharacterEquipper.generated.h"


class AShooterCharacter;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UShooterCharacterEquipper : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class REIGNFORCE_API IShooterCharacterEquipper
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ShooterCharacterEquipper")
	void ProvideWithAvailableWeapons(AShooterCharacter* ShooterCharacter);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ShooterCharacterEquipper")
	void ProvideWithRanksProgression(AShooterCharacter* ShooterCharacter);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ShooterCharacterEquipper")
	void AdjustBehavior(AShooterCharacter* ShooterCharacter);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ShooterCharacterEquipper")
	void DistributeBehaviors(const TArray<AShooterCharacter*>& ShooterCharacters);
};
