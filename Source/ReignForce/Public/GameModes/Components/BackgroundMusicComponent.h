// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BackgroundMusicComponent.generated.h"


class UAudioComponent;
class UGameMusicDataAsset;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REIGNFORCE_API UBackgroundMusicComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBackgroundMusicComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure)
	bool IsDataAssetValid() const;

	UFUNCTION(BlueprintPure)
	bool CanLoadMusicSound() const;

	UFUNCTION(BlueprintCallable)
	void ResetActiveGameplaySound();

	UFUNCTION(BlueprintCallable)
	void ResetActiveMainMenuSound();

	UFUNCTION(BlueprintCallable)
	void ResetActiveSkillsMenuSound();

	UFUNCTION(BlueprintCallable)
	void ResetActivePauseMenuSound();

	UFUNCTION(BlueprintCallable)
	void SetPauseForActiveGameplaySound(bool bPause);

	UFUNCTION(BlueprintCallable)
	void SetPauseForActiveMainMenuSound(bool bPause);

	UFUNCTION(BlueprintCallable)
	void SetPauseForActiveSkillsMenuSound(bool bPause);

	UFUNCTION(BlueprintCallable)
	void StopActiveGameplaySound();

	UFUNCTION(BlueprintCallable)
	void StopActiveMainMenuSound();

	UFUNCTION(BlueprintCallable)
	void StopActiveSkillsMenuSound();

	UFUNCTION(BlueprintCallable)
	void StopActivePauseMenuSound();

	UFUNCTION(BlueprintCallable)
	void StopAllActiveMusic();

	FORCEINLINE UAudioComponent* GetActiveGameplaySound() const { return ActiveGameplaySound; }
	FORCEINLINE UAudioComponent* GetActiveMainMenuSound() const { return ActiveMainMenuSound; }
	FORCEINLINE UAudioComponent* GetActiveSkillsMenuSound() const { return ActiveSkillsMenuSound; }
	FORCEINLINE UAudioComponent* GetActivePauseMenuSound() const { return ActivePauseMenuSound; }

protected:
	virtual void BeginDestroy() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music", meta = (AllowPrivateAccess = true))
	TObjectPtr<UGameMusicDataAsset> GameMusicDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Times"))
	float ActiveGameplayVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Times"))
	float ActiveMainMenuVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Times"))
	float ActiveSkillsMenuVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Music", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Times"))
	float ActivePauseMenuVolume;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAudioComponent> ActiveGameplaySound;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAudioComponent> ActiveMainMenuSound;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAudioComponent> ActiveSkillsMenuSound;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAudioComponent> ActivePauseMenuSound;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	int32 ActiveGameplaySoundIndex;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	int32 ActiveMainMenuSoundIndex;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	int32 ActiveSkillsMenuSoundIndex;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	int32 ActivePauseMenuSoundIndex;

	void LoadActiveGameplaySound(int32 NextToPlay);
	void LoadActiveMainMenuSound(int32 NextToPlay);
	void LoadActiveSkillsMenuSound(int32 NextToPlay);
	void LoadActivePauseMenuSound(int32 NextToPlay);
};
