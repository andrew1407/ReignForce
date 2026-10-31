// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/GameSettingsSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Settings/ReignForceGameUserSettings.h"


void UGameSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    WorldInitializedHandle = FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &UGameSettingsSubsystem::HandleWorldInitializedActors);
}

void UGameSettingsSubsystem::Deinitialize()
{
    FWorldDelegates::OnWorldInitializedActors.Remove(WorldInitializedHandle);

    Super::Deinitialize();
}

void UGameSettingsSubsystem::ApplyAudioSettings() const
{
    UReignForceGameUserSettings* Settings = UReignForceGameUserSettings::GetReignForceGameUserSettings();
    if (IsValid(Settings)) Settings->ApplyAudioSettings(GetGameInstance());
}

void UGameSettingsSubsystem::HandleWorldInitializedActors(const UWorld::FActorsInitializedParams& Params)
{
    bool bOwnWorld = IsValid(Params.World) && Params.World->GetGameInstance() == GetGameInstance();
    if (!bOwnWorld) return;
    UReignForceGameUserSettings* Settings = UReignForceGameUserSettings::GetReignForceGameUserSettings();
    if (IsValid(Settings)) Settings->ApplyAudioSettings(Params.World);
}
