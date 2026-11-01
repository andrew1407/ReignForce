// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/ReignForceGameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"

#include "Settings/ReignForceSettings.h"


UReignForceGameUserSettings::UReignForceGameUserSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    MasterVolume = 1;
    MusicVolume = 1;
    EffectsVolume = 1;
    LookSensitivity = 1;
    bInvertMouseX = bInvertMouseY = bInvertGamepadX = bInvertGamepadY = false;
}

UReignForceGameUserSettings* UReignForceGameUserSettings::GetReignForceGameUserSettings()
{
    return Cast<UReignForceGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}

void UReignForceGameUserSettings::SetToDefaults()
{
    Super::SetToDefaults();

    // The engine default is borderless (r.FullScreenMode), the game starts in the real fullscreen at the desktop resolution
    FullscreenMode = EWindowMode::Fullscreen;
    LastConfirmedFullscreenMode = EWindowMode::Fullscreen;
    PreferredFullscreenMode = EWindowMode::Fullscreen;

    MasterVolume = 1;
    MusicVolume = 1;
    EffectsVolume = 1;
    LookSensitivity = 1;
    bInvertMouseX = bInvertMouseY = bInvertGamepadX = bInvertGamepadY = false;
}

void UReignForceGameUserSettings::ApplyAudioSettings(const UObject* WorldContextObject) const
{
    const UReignForceSettings* Settings = UReignForceSettings::Get();
    if (!IsValid(WorldContextObject) || !Settings) return;

    USoundMix* SoundMix = Settings->UserSoundMix.LoadSynchronous();
    if (!IsValid(SoundMix)) return;

    constexpr float Pitch = 1;
    constexpr float FadeInTime = 0;
    constexpr bool bApplyToChildren = true;

    USoundClass* MasterClass = Settings->MasterSoundClass.LoadSynchronous();
    if (IsValid(MasterClass))
        UGameplayStatics::SetSoundMixClassOverride(WorldContextObject, SoundMix, MasterClass, MasterVolume * EffectsVolume, Pitch, FadeInTime, bApplyToChildren);

    USoundClass* MusicClass = Settings->MusicSoundClass.LoadSynchronous();
    if (IsValid(MusicClass))
        UGameplayStatics::SetSoundMixClassOverride(WorldContextObject, SoundMix, MusicClass, MasterVolume * MusicVolume, Pitch, FadeInTime, bApplyToChildren);

    // Re-push keeps a single mix reference on the audio device
    UGameplayStatics::PopSoundMixModifier(WorldContextObject, SoundMix);
    UGameplayStatics::PushSoundMixModifier(WorldContextObject, SoundMix);
}

void UReignForceGameUserSettings::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.f, 1.f);
}

void UReignForceGameUserSettings::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.f, 1.f);
}

void UReignForceGameUserSettings::SetEffectsVolume(float Volume)
{
    EffectsVolume = FMath::Clamp(Volume, 0.f, 1.f);
}

void UReignForceGameUserSettings::SetLookSensitivity(float Sensitivity)
{
    LookSensitivity = FMath::Clamp(Sensitivity, .1f, 3.f);
}


bool UReignForceGameUserSettings::IsLookInverted(bool bGamepad, bool bVertical) const
{
    if (bGamepad) return bVertical ? bInvertGamepadY : bInvertGamepadX;
    return bVertical ? bInvertMouseY : bInvertMouseX;
}

void UReignForceGameUserSettings::SetLookInverted(bool bGamepad, bool bVertical, bool bInverted)
{
    bool& Flag = bGamepad ? (bVertical ? bInvertGamepadY : bInvertGamepadX) : (bVertical ? bInvertMouseY : bInvertMouseX);
    Flag = bInverted;
}

FVector2D UReignForceGameUserSettings::ApplyLookInversion(const FVector2D& LookInput, bool bGamepad) const
{
    constexpr bool bVertical = true;
    return FVector2D(
        IsLookInverted(bGamepad, !bVertical) ? -LookInput.X : LookInput.X,
        IsLookInverted(bGamepad, bVertical) ? -LookInput.Y : LookInput.Y
    );
}
