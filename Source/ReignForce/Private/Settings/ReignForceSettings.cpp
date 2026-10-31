// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/ReignForceSettings.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Engine/Texture2D.h"


UReignForceSettings::UReignForceSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    UserSoundMix = TSoftObjectPtr<USoundMix>(FSoftObjectPath(TEXT("/Game/Core/Audio/SM_UserSettings.SM_UserSettings")));
    MasterSoundClass = TSoftObjectPtr<USoundClass>(FSoftObjectPath(TEXT("/Engine/EngineSounds/Master.Master")));
    MusicSoundClass = TSoftObjectPtr<USoundClass>(FSoftObjectPath(TEXT("/Game/Core/Audio/SC_Music.SC_Music")));

    LoadingScreenImage = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/Core/UI/MainMenu/Images/main-menu-image.main-menu-image")));
    MinLoadingScreenDisplayTime = .5f;
}
