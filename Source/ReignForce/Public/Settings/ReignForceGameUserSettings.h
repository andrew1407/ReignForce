// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "ReignForceGameUserSettings.generated.h"


/**
 * Engine user settings (resolution, window mode, quality) extended with the audio volumes
 */
UCLASS()
class REIGNFORCE_API UReignForceGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UReignForceGameUserSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure, Category = "Settings")
	static UReignForceGameUserSettings* GetReignForceGameUserSettings();

	virtual void SetToDefaults() override;

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void ApplyAudioSettings(const UObject* WorldContextObject) const;

	UFUNCTION(BlueprintPure, Category = "Settings|Audio")
	FORCEINLINE float GetMasterVolume() const { return MasterVolume; }

	UFUNCTION(BlueprintPure, Category = "Settings|Audio")
	FORCEINLINE float GetMusicVolume() const { return MusicVolume; }

	UFUNCTION(BlueprintPure, Category = "Settings|Audio")
	FORCEINLINE float GetEffectsVolume() const { return EffectsVolume; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetMusicVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetEffectsVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "Settings|Controls")
	FORCEINLINE float GetLookSensitivity() const { return LookSensitivity; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void SetLookSensitivity(float Sensitivity);

private:
	UPROPERTY(Config)
	float MasterVolume;

	UPROPERTY(Config)
	float MusicVolume;

	UPROPERTY(Config)
	float EffectsVolume;

	UPROPERTY(Config)
	float LookSensitivity;
};
