// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerCharacter/PlayerInteractionMode.h"
#include "ShooterPlayerController.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetInputModeUI();

	UFUNCTION(BlueprintCallable)
	void SetInputModeGameplay();

	UFUNCTION(BlueprintCallable)
	bool SetInteractionMode(EPlayerInteractionMode Mode);
};
