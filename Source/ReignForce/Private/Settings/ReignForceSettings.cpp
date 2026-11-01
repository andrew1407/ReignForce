// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/ReignForceSettings.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Engine/Texture2D.h"
#include "Weapons/ImpactEffectsDataAsset.h"
#include "InputMappingContext.h"


UReignForceSettings::UReignForceSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    UserSoundMix = TSoftObjectPtr<USoundMix>(FSoftObjectPath(TEXT("/Game/Core/Audio/SM_UserSettings.SM_UserSettings")));
    MasterSoundClass = TSoftObjectPtr<USoundClass>(FSoftObjectPath(TEXT("/Engine/EngineSounds/Master.Master")));
    MusicSoundClass = TSoftObjectPtr<USoundClass>(FSoftObjectPath(TEXT("/Game/Core/Audio/SC_Music.SC_Music")));

    LoadingScreenImage = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Core/UI/Loading/T_LoadingScreen.T_LoadingScreen")));
    MinLoadingScreenDisplayTime = .5f;

    ImpactEffects = TSoftObjectPtr<UImpactEffectsDataAsset>(FSoftObjectPath(TEXT("/Game/Core/Weapons/Impacts/DA_ImpactEffects.DA_ImpactEffects")));

    RemappableInputContext = TSoftObjectPtr<UInputMappingContext>(FSoftObjectPath(TEXT("/Game/ThirdPerson/Input/IMC_Default.IMC_Default")));
    GamepadSchemeImage = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Core/UI/Settings/T_GamepadScheme.T_GamepadScheme")));
}
