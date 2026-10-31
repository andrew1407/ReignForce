// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ReignForceSettings.generated.h"


class USoundMix;
class USoundClass;
class UTexture2D;
class UImpactEffectsDataAsset;
class UInputMappingContext;

/**
 * Project wide assets used by the game systems (Project Settings -> Game -> Reign Force)
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Reign Force"))
class REIGNFORCE_API UReignForceSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UReignForceSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static const UReignForceSettings* Get() { return GetDefault<UReignForceSettings>(); }

	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	#pragma region AUDIO

	/** Mix pushed with the user volume overrides */
	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	TSoftObjectPtr<USoundMix> UserSoundMix;

	/** Root class of all the game sounds (effects, UI, voices) */
	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	TSoftObjectPtr<USoundClass> MasterSoundClass;

	/** Background music class, kept apart from the master one to be tuned separately */
	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	TSoftObjectPtr<USoundClass> MusicSoundClass;

	#pragma endregion

	UPROPERTY(Config, EditAnywhere, Category = "Loading")
	TSoftObjectPtr<UTexture2D> LoadingScreenImage;

	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (ClampMin = 0, UIMin = 0, Units = "Seconds"))
	float MinLoadingScreenDisplayTime;

	/** Surface hit particles & marks (bullet holes, blade cuts) */
	UPROPERTY(Config, EditAnywhere, Category = "Weapons")
	TSoftObjectPtr<UImpactEffectsDataAsset> ImpactEffects;

	/** Gameplay input context which keyboard & mouse mappings are listed in the controls menu for remapping */
	UPROPERTY(Config, EditAnywhere, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> RemappableInputContext;
};
