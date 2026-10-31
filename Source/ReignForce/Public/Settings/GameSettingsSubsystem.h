// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameSettingsSubsystem.generated.h"


/**
 * Applies the saved user settings (audio mix) to every world the game opens
 */
UCLASS()
class REIGNFORCE_API UGameSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyAudioSettings() const;

private:
	FDelegateHandle WorldInitializedHandle;

	void HandleWorldInitializedActors(const UWorld::FActorsInitializedParams& Params);
};
