// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/Components/BackgroundMusicComponent.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "GameModes/Components/GameMusicDataAsset.h"


namespace
{
	int32 FindRandomSoundFromBunch(const TArray<TSoftObjectPtr<USoundBase>>& Bunch, int32 CurrentIndex)
	{
		TArray<TSoftObjectPtr<USoundBase>> ValidSounds = Bunch.FilterByPredicate([&] (TSoftObjectPtr<USoundBase> Sound)
		{
			bool bCurrentIndexValid = !Bunch.IsValidIndex(CurrentIndex) || CurrentIndex != Bunch.IndexOfByKey(Sound);
			return !Sound.IsNull() && bCurrentIndexValid;
		});
		if (ValidSounds.IsEmpty()) return INDEX_NONE;
		TSoftObjectPtr<USoundBase>& Sound = ValidSounds[FMath::RandRange(0, ValidSounds.Num() - 1)];
		return Bunch.IndexOfByKey(Sound);
	}
}

UBackgroundMusicComponent::UBackgroundMusicComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ActiveGameplayVolume = .65f;
	ActiveMainMenuVolume = 1;
	ActiveSkillsMenuVolume = 1;
	ActivePauseMenuVolume = 1;

	ActiveGameplaySoundIndex = INDEX_NONE;
	ActiveMainMenuSoundIndex = INDEX_NONE;
	ActiveSkillsMenuSoundIndex = INDEX_NONE;
	ActivePauseMenuSoundIndex = INDEX_NONE;
}

void UBackgroundMusicComponent::BeginDestroy()
{
	StopAllActiveMusic();

	Super::BeginDestroy();
}

bool UBackgroundMusicComponent::IsDataAssetValid() const
{
	return IsValid(GameMusicDataAsset) && GameMusicDataAsset->IsValidLowLevel();
}

bool UBackgroundMusicComponent::CanLoadMusicSound() const
{
	return IsValid(this) && IsDataAssetValid();
}

void UBackgroundMusicComponent::ResetActiveGameplaySound()
{
	if (!CanLoadMusicSound()) return;

	int32 NextToPlay = FindRandomSoundFromBunch(GameMusicDataAsset->GamepayMusic, ActiveGameplaySoundIndex);
	if (!GameMusicDataAsset->GamepayMusic.IsValidIndex(NextToPlay)) NextToPlay = ActiveGameplaySoundIndex;
	if (!GameMusicDataAsset->GamepayMusic.IsValidIndex(NextToPlay)) return;
	TSoftObjectPtr<USoundBase>& ToLoad = GameMusicDataAsset->GamepayMusic[NextToPlay];
	if (ToLoad.IsNull()) return;

	FSimpleDelegate OnLoad = FStreamableDelegate::CreateUObject(this, &UBackgroundMusicComponent::LoadActiveGameplaySound, NextToPlay);
	UAssetManager::GetStreamableManager().RequestAsyncLoad(ToLoad.ToSoftObjectPath(), MoveTemp(OnLoad));
}

void UBackgroundMusicComponent::ResetActiveMainMenuSound()
{
	if (!CanLoadMusicSound()) return;

	int32 NextToPlay = FindRandomSoundFromBunch(GameMusicDataAsset->MainMenuMusic, ActiveMainMenuSoundIndex);
	if (!GameMusicDataAsset->MainMenuMusic.IsValidIndex(NextToPlay)) NextToPlay = ActiveMainMenuSoundIndex;
	if (!GameMusicDataAsset->MainMenuMusic.IsValidIndex(NextToPlay)) return;
	TSoftObjectPtr<USoundBase>& ToLoad = GameMusicDataAsset->MainMenuMusic[NextToPlay];
	if (ToLoad.IsNull()) return;

	FSimpleDelegate OnLoad = FStreamableDelegate::CreateUObject(this, &UBackgroundMusicComponent::LoadActiveMainMenuSound, NextToPlay);
	UAssetManager::GetStreamableManager().RequestAsyncLoad(ToLoad.ToSoftObjectPath(), MoveTemp(OnLoad));
}

void UBackgroundMusicComponent::ResetActiveSkillsMenuSound()
{
	if (!CanLoadMusicSound()) return;

	int32 NextToPlay = FindRandomSoundFromBunch(GameMusicDataAsset->SkillsMenuMusic, ActiveSkillsMenuSoundIndex);
	if (!GameMusicDataAsset->SkillsMenuMusic.IsValidIndex(NextToPlay)) NextToPlay = ActiveSkillsMenuSoundIndex;
	if (!GameMusicDataAsset->SkillsMenuMusic.IsValidIndex(NextToPlay)) return;
	TSoftObjectPtr<USoundBase>& ToLoad = GameMusicDataAsset->SkillsMenuMusic[NextToPlay];
	if (ToLoad.IsNull()) return;

	FSimpleDelegate OnLoad = FStreamableDelegate::CreateUObject(this, &UBackgroundMusicComponent::LoadActiveSkillsMenuSound, NextToPlay);
	UAssetManager::GetStreamableManager().RequestAsyncLoad(ToLoad.ToSoftObjectPath(), MoveTemp(OnLoad));
}

void UBackgroundMusicComponent::ResetActivePauseMenuSound()
{
	if (!CanLoadMusicSound()) return;

	int32 NextToPlay = FindRandomSoundFromBunch(GameMusicDataAsset->PauseMenuMusic, ActivePauseMenuSoundIndex);
	if (!GameMusicDataAsset->PauseMenuMusic.IsValidIndex(NextToPlay)) NextToPlay = ActivePauseMenuSoundIndex;
	if (!GameMusicDataAsset->PauseMenuMusic.IsValidIndex(NextToPlay)) return;
	TSoftObjectPtr<USoundBase>& ToLoad = GameMusicDataAsset->PauseMenuMusic[NextToPlay];
	if (ToLoad.IsNull()) return;

	FSimpleDelegate OnLoad = FStreamableDelegate::CreateUObject(this, &UBackgroundMusicComponent::LoadActivePauseMenuSound, NextToPlay);
	UAssetManager::GetStreamableManager().RequestAsyncLoad(ToLoad.ToSoftObjectPath(), MoveTemp(OnLoad));
}

void UBackgroundMusicComponent::SetPauseForActiveGameplaySound(bool bPause)
{
	if (IsValid(ActiveGameplaySound)) ActiveGameplaySound->SetPaused(bPause);
}

void UBackgroundMusicComponent::SetPauseForActiveMainMenuSound(bool bPause)
{
	if (IsValid(ActiveMainMenuSound)) ActiveMainMenuSound->SetPaused(bPause);
}

void UBackgroundMusicComponent::SetPauseForActiveSkillsMenuSound(bool bPause)
{
	if (IsValid(ActiveSkillsMenuSound)) ActiveSkillsMenuSound->SetPaused(bPause);
}

void UBackgroundMusicComponent::StopActiveGameplaySound()
{
	if (!IsValid(ActiveGameplaySound)) return;
	ActiveGameplaySound->OnAudioFinished.RemoveDynamic(this, &UBackgroundMusicComponent::ResetActiveGameplaySound);
	ActiveGameplaySound->Stop();
}

void UBackgroundMusicComponent::StopActiveMainMenuSound()
{
	if (!IsValid(ActiveMainMenuSound)) return;
	ActiveMainMenuSound->OnAudioFinished.RemoveDynamic(this, &UBackgroundMusicComponent::ResetActiveMainMenuSound);
	ActiveMainMenuSound->Stop();
}

void UBackgroundMusicComponent::StopActiveSkillsMenuSound()
{
	if (!IsValid(ActiveSkillsMenuSound)) return;
	ActiveSkillsMenuSound->OnAudioFinished.RemoveDynamic(this, &UBackgroundMusicComponent::ResetActiveSkillsMenuSound);
	ActiveSkillsMenuSound->Stop();
}

void UBackgroundMusicComponent::StopActivePauseMenuSound()
{
	if (!IsValid(ActivePauseMenuSound)) return;
	ActivePauseMenuSound->OnAudioFinished.RemoveDynamic(this, &UBackgroundMusicComponent::ResetActivePauseMenuSound);
	ActivePauseMenuSound->Stop();
}

void UBackgroundMusicComponent::StopAllActiveMusic()
{
	StopActiveGameplaySound();
	StopActiveMainMenuSound();
	StopActiveSkillsMenuSound();
	StopActivePauseMenuSound();
}

void UBackgroundMusicComponent::LoadActiveGameplaySound(int32 NextToPlay)
{
	if (!CanLoadMusicSound()) return;
	if (!GameMusicDataAsset->GamepayMusic.IsValidIndex(NextToPlay)) return;

	TSoftObjectPtr<USoundBase>& Loaded = GameMusicDataAsset->GamepayMusic[NextToPlay];
	if (!Loaded.IsValid()) return;

	constexpr float PitchMultiplier = 1;
	UAudioComponent* NewSound = UGameplayStatics::SpawnSound2D(GetWorld(), Loaded.Get(), ActiveGameplayVolume, PitchMultiplier);
	ActiveGameplaySoundIndex = NextToPlay;

	if (!IsValid(NewSound)) return;
	StopActiveGameplaySound();
	ActiveGameplaySound = NewSound;

	if (IsValid(ActiveGameplaySound))
		ActiveGameplaySound->OnAudioFinished.AddDynamic(this, &UBackgroundMusicComponent::ResetActiveGameplaySound);
}

void UBackgroundMusicComponent::LoadActiveMainMenuSound(int32 NextToPlay)
{
	if (!CanLoadMusicSound()) return;
	if (!GameMusicDataAsset->MainMenuMusic.IsValidIndex(NextToPlay)) return;

	TSoftObjectPtr<USoundBase>& Loaded = GameMusicDataAsset->MainMenuMusic[NextToPlay];
	if (!Loaded.IsValid()) return;

	constexpr float PitchMultiplier = 1;
	UAudioComponent* NewSound = UGameplayStatics::SpawnSound2D(GetWorld(), Loaded.Get(), ActiveMainMenuVolume, PitchMultiplier);
	ActiveMainMenuSoundIndex = NextToPlay;

	if (!IsValid(NewSound)) return;
	if (IsValid(ActiveMainMenuSound))
	StopActiveMainMenuSound();
	ActiveMainMenuSound = NewSound;

	if (IsValid(ActiveMainMenuSound))
		ActiveMainMenuSound->OnAudioFinished.AddDynamic(this, &UBackgroundMusicComponent::ResetActiveMainMenuSound);
}

void UBackgroundMusicComponent::LoadActiveSkillsMenuSound(int32 NextToPlay)
{
	if (!CanLoadMusicSound()) return;
	if (!GameMusicDataAsset->SkillsMenuMusic.IsValidIndex(NextToPlay)) return;

	TSoftObjectPtr<USoundBase>& Loaded = GameMusicDataAsset->SkillsMenuMusic[NextToPlay];
	if (!Loaded.IsValid()) return;

	constexpr float PitchMultiplier = 1;
	UAudioComponent* NewSound = UGameplayStatics::SpawnSound2D(GetWorld(), Loaded.Get(), ActiveSkillsMenuVolume, PitchMultiplier);
	ActiveSkillsMenuSoundIndex = NextToPlay;

	if (!IsValid(NewSound)) return;
	StopActiveSkillsMenuSound();
	ActiveSkillsMenuSound = NewSound;

	if (IsValid(ActiveSkillsMenuSound))
		ActiveSkillsMenuSound->OnAudioFinished.AddDynamic(this, &UBackgroundMusicComponent::ResetActiveSkillsMenuSound);
}

void UBackgroundMusicComponent::LoadActivePauseMenuSound(int32 NextToPlay)
{
	if (!CanLoadMusicSound()) return;
	if (!GameMusicDataAsset->PauseMenuMusic.IsValidIndex(NextToPlay)) return;

	TSoftObjectPtr<USoundBase>& Loaded = GameMusicDataAsset->PauseMenuMusic[NextToPlay];
	if (!Loaded.IsValid()) return;

	constexpr float PitchMultiplier = 1;
	UAudioComponent* NewSound = UGameplayStatics::SpawnSound2D(GetWorld(), Loaded.Get(), ActivePauseMenuVolume, PitchMultiplier);
	ActivePauseMenuSoundIndex = NextToPlay;

	if (!IsValid(NewSound)) return;
	StopActivePauseMenuSound();
	ActivePauseMenuSound = NewSound;

	if (IsValid(ActivePauseMenuSound))
		ActivePauseMenuSound->OnAudioFinished.AddDynamic(this, &UBackgroundMusicComponent::ResetActivePauseMenuSound);
}
