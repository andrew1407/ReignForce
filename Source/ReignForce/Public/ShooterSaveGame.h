// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ShooterCharacter/Stats/RanksProgression.h"
#include "Weapons/WeaponType.h"
#include "GameStates/Components/PlayerProgressionBalance.h"
#include "ShooterSaveGame.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API UShooterSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charater")
	FRanksProgression RanksProgression;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charater|Position")
	FVector ShooterPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charater|Position")
	FRotator CameraRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Charater|Skills")
	FPlayerProgressionBalance PlayerProgressionBalance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TMap<EWeaponType, int32> WeaponModels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSet<EWeaponType> AvailableSlots;
};
