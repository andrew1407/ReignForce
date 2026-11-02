// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/GameSettingsOptions.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Algo/Find.h"
#include "Settings/ReignForceGameUserSettings.h"


namespace
{
    constexpr int32 VOLUME_STEPS = 10;
    constexpr int32 SENSITIVITY_MIN_PERCENT = 20;
    constexpr int32 SENSITIVITY_MAX_PERCENT = 300;
    constexpr int32 SENSITIVITY_PERCENT_STEP = 10;
    constexpr int32 DEFAULT_QUALITY_LEVEL = 3;
    constexpr int32 CUSTOM_QUALITY_LEVEL = -1;
    const FIntPoint DEFAULT_RESOLUTION(1920, 1080);

    using FVolumeGetter = float (UReignForceGameUserSettings::*)() const;
    using FVolumeSetter = void (UReignForceGameUserSettings::*)(float);

    struct FVolumeOption
    {
        EGameSettingsOption Option;
        const TCHAR* Label;
        FVolumeGetter Get;
        FVolumeSetter Set;
    };

    /** Look axis inversion rows: the device (mouse or gamepad stick) & the axis they flip */
    struct FInversionOption
    {
        EGameSettingsOption Option;
        const TCHAR* Label;
        bool bGamepad;
        bool bVertical;
    };

    const FVolumeOption VOLUME_OPTIONS[] = {
        { EGameSettingsOption::MASTER_VOLUME, TEXT("Master volume"), &UReignForceGameUserSettings::GetMasterVolume, &UReignForceGameUserSettings::SetMasterVolume },
        { EGameSettingsOption::MUSIC_VOLUME, TEXT("Music volume"), &UReignForceGameUserSettings::GetMusicVolume, &UReignForceGameUserSettings::SetMusicVolume },
        { EGameSettingsOption::EFFECTS_VOLUME, TEXT("Game sounds volume"), &UReignForceGameUserSettings::GetEffectsVolume, &UReignForceGameUserSettings::SetEffectsVolume },
    };

    // Inversion flips the default direction of the axis (camera turn / tilt) for its device
    const FInversionOption INVERSION_OPTIONS[] = {
        { EGameSettingsOption::INVERT_MOUSE_X, TEXT("Invert mouse left / right"), false, false },
        { EGameSettingsOption::INVERT_MOUSE_Y, TEXT("Invert mouse up / down"), false, true },
        { EGameSettingsOption::INVERT_GAMEPAD_X, TEXT("Invert gamepad stick left / right"), true, false },
        { EGameSettingsOption::INVERT_GAMEPAD_Y, TEXT("Invert gamepad stick up / down"), true, true },
    };

    FText Text(const TCHAR* Value) { return FText::FromString(Value); }
    FText Percent(int32 Value) { return FText::FromString(FString::Printf(TEXT("%d%%"), Value)); }

    FGameSettingsOptionRow MakeSwitchRow(const TCHAR* Label)
    {
        return FGameSettingsOptionRow::Of(Text(Label), { Text(TEXT("Off")), Text(TEXT("On")) }, { 0, 1 });
    }
}

FGameSettingsOptionRow FGameSettingsOptionRow::Of(const FText& Label, TArray<FText> Names, TArray<int32> Values, bool bLive)
{
    FGameSettingsOptionRow Row;
    Row.Label = Label;
    Row.Names = MoveTemp(Names);
    Row.Values = MoveTemp(Values);
    Row.bLive = bLive;
    return Row;
}

void FGameSettingsOptionRow::Select(int32 Value)
{
    int32 Closest = 0;
    for (int32 i = 0; i < Values.Num(); ++i)
        if (FMath::Abs(Values[i] - Value) < FMath::Abs(Values[Closest] - Value)) Closest = i;
    Index = Closest;
}

FGameSettingsOptions FGameSettingsOptions::Of(const UReignForceGameUserSettings* Settings)
{
    FGameSettingsOptions Options;
    Options.Rows.SetNum(static_cast<int32>(EGameSettingsOption::COUNT));

    Options.GetMutableRow(EGameSettingsOption::WINDOW_MODE) = FGameSettingsOptionRow::Of(Text(TEXT("Window mode")),
        { Text(TEXT("Fullscreen")), Text(TEXT("Borderless")), Text(TEXT("Windowed")) },
        { EWindowMode::Fullscreen, EWindowMode::WindowedFullscreen, EWindowMode::Windowed });

    Options.ReadResolutions(IsValid(Settings) ? Settings->GetScreenResolution() : DEFAULT_RESOLUTION);

    FGameSettingsOptionRow& Quality = Options.GetMutableRow(EGameSettingsOption::QUALITY) = FGameSettingsOptionRow::Of(Text(TEXT("Graphics quality")),
        { Text(TEXT("Low")), Text(TEXT("Medium")), Text(TEXT("High")), Text(TEXT("Epic")), Text(TEXT("Cinematic")) },
        { 0, 1, 2, 3, 4 });
    // Per-group custom scalability is kept untouched until another level is picked
    if (IsValid(Settings) && Settings->GetOverallScalabilityLevel() < 0)
    {
        Quality.Names.Insert(Text(TEXT("Custom")), 0);
        Quality.Values.Insert(CUSTOM_QUALITY_LEVEL, 0);
    }

    Options.GetMutableRow(EGameSettingsOption::VSYNC) = MakeSwitchRow(TEXT("VSync"));

    FGameSettingsOptionRow& FrameRate = Options.GetMutableRow(EGameSettingsOption::FRAME_RATE) = FGameSettingsOptionRow::Of(Text(TEXT("Frame rate limit")), {}, { 30, 60, 120, 144, 240, 0 });
    for (int32 Limit : FrameRate.Values) FrameRate.Names.Add(Limit > 0 ? FText::AsNumber(Limit) : Text(TEXT("Unlimited")));

    FGameSettingsOptionRow& Sensitivity = Options.GetMutableRow(EGameSettingsOption::LOOK_SENSITIVITY) = FGameSettingsOptionRow::Of(Text(TEXT("Camera sensitivity")), {}, {});
    for (int32 Value = SENSITIVITY_MIN_PERCENT; Value <= SENSITIVITY_MAX_PERCENT; Value += SENSITIVITY_PERCENT_STEP)
    {
        Sensitivity.Values.Add(Value);
        Sensitivity.Names.Add(Percent(Value));
    }

    for (const FInversionOption& Inversion : INVERSION_OPTIONS)
        Options.GetMutableRow(Inversion.Option) = MakeSwitchRow(Inversion.Label);

    constexpr bool bLive = true;
    for (const FVolumeOption& Volume : VOLUME_OPTIONS)
    {
        FGameSettingsOptionRow& Row = Options.GetMutableRow(Volume.Option) = FGameSettingsOptionRow::Of(Text(Volume.Label), {}, {}, bLive);
        for (int32 Step = 0; Step <= VOLUME_STEPS; ++Step)
        {
            Row.Values.Add(Step);
            Row.Names.Add(Percent(Step * 100 / VOLUME_STEPS));
        }
    }

    if (IsValid(Settings)) Options.SelectCurrentValues(Settings);
    return Options;
}

void FGameSettingsOptions::ReadResolutions(const FIntPoint& Current)
{
    UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
    Resolutions.AddUnique(Current);
    Resolutions.Sort([] (const FIntPoint& A, const FIntPoint& B) { return A.X == B.X ? A.Y < B.Y : A.X < B.X; });

    FGameSettingsOptionRow& Row = GetMutableRow(EGameSettingsOption::RESOLUTION) = FGameSettingsOptionRow::Of(Text(TEXT("Resolution")), {}, {});
    for (int32 i = 0; i < Resolutions.Num(); ++i)
    {
        Row.Names.Add(FText::FromString(FString::Printf(TEXT("%d x %d"), Resolutions[i].X, Resolutions[i].Y)));
        Row.Values.Add(i);
    }
}

void FGameSettingsOptions::SelectCurrentValues(const UReignForceGameUserSettings* Settings)
{
    GetMutableRow(EGameSettingsOption::WINDOW_MODE).Select(Settings->GetFullscreenMode());
    GetMutableRow(EGameSettingsOption::RESOLUTION).Select(Resolutions.IndexOfByKey(Settings->GetScreenResolution()));
    GetMutableRow(EGameSettingsOption::QUALITY).Select(Settings->GetOverallScalabilityLevel());
    GetMutableRow(EGameSettingsOption::VSYNC).Select(Settings->IsVSyncEnabled() ? 1 : 0);

    // No limit is the last value
    FGameSettingsOptionRow& FrameRate = GetMutableRow(EGameSettingsOption::FRAME_RATE);
    const int32 FrameRateLimit = FMath::RoundToInt(Settings->GetFrameRateLimit());
    if (FrameRateLimit > 0) FrameRate.Select(FrameRateLimit);
    else FrameRate.Index = FrameRate.Values.Num() - 1;

    GetMutableRow(EGameSettingsOption::LOOK_SENSITIVITY).Select(FMath::RoundToInt(Settings->GetLookSensitivity() * 100));
    for (const FInversionOption& Inversion : INVERSION_OPTIONS)
        GetMutableRow(Inversion.Option).Select(Settings->IsLookInverted(Inversion.bGamepad, Inversion.bVertical) ? 1 : 0);
    for (const FVolumeOption& Volume : VOLUME_OPTIONS)
        GetMutableRow(Volume.Option).Select(FMath::RoundToInt((Settings->*Volume.Get)() * VOLUME_STEPS));
    MarkApplied();
}

bool FGameSettingsOptions::Change(EGameSettingsOption Option, int32 Delta, bool bWrap)
{
    if (!Rows.IsValidIndex(static_cast<int32>(Option))) return false;
    FGameSettingsOptionRow& Row = GetMutableRow(Option);
    const int32 Count = Row.Values.Num();
    if (Count == 0) return false;

    const int32 Next = Row.Index + Delta;
    const int32 Index = bWrap ? (Next % Count + Count) % Count : FMath::Clamp(Next, 0, Count - 1);
    if (Index == Row.Index) return false;
    Row.Index = Index;
    return true;
}

void FGameSettingsOptions::WriteTo(UReignForceGameUserSettings* Settings) const
{
    if (!IsValid(Settings)) return;

    Settings->SetFullscreenMode(static_cast<EWindowMode::Type>(GetRow(EGameSettingsOption::WINDOW_MODE).GetValue()));
    const int32 ResolutionIndex = GetRow(EGameSettingsOption::RESOLUTION).GetValue();
    if (Resolutions.IsValidIndex(ResolutionIndex)) Settings->SetScreenResolution(Resolutions[ResolutionIndex]);
    const int32 QualityLevel = GetRow(EGameSettingsOption::QUALITY).GetValue();
    if (QualityLevel != CUSTOM_QUALITY_LEVEL) Settings->SetOverallScalabilityLevel(QualityLevel);
    Settings->SetVSyncEnabled(GetRow(EGameSettingsOption::VSYNC).GetValue() != 0);
    Settings->SetFrameRateLimit(GetRow(EGameSettingsOption::FRAME_RATE).GetValue());
    Settings->SetLookSensitivity(GetRow(EGameSettingsOption::LOOK_SENSITIVITY).GetValue() / 100.f);
    for (const FInversionOption& Inversion : INVERSION_OPTIONS)
        Settings->SetLookInverted(Inversion.bGamepad, Inversion.bVertical, GetRow(Inversion.Option).GetValue() != 0);
    for (const FVolumeOption& Volume : VOLUME_OPTIONS) PreviewTo(Settings, Volume.Option);
}

void FGameSettingsOptions::PreviewTo(UReignForceGameUserSettings* Settings, EGameSettingsOption Option) const
{
    const FVolumeOption* Volume = Algo::FindBy(VOLUME_OPTIONS, Option, &FVolumeOption::Option);
    if (!(IsValid(Settings) && Volume)) return;
    (Settings->*Volume->Set)(GetRow(Option).GetValue() / static_cast<float>(VOLUME_STEPS));
}

void FGameSettingsOptions::MarkApplied()
{
    for (FGameSettingsOptionRow& Row : Rows) Row.AppliedIndex = Row.Index;
}

bool FGameSettingsOptions::HasPendingChanges() const
{
    return Rows.ContainsByPredicate([] (const FGameSettingsOptionRow& Row) { return Row.IsChanged(); });
}

const FGameSettingsOptionRow& FGameSettingsOptions::GetRow(EGameSettingsOption Option) const
{
    return Rows[static_cast<int32>(Option)];
}

FGameSettingsOptionRow& FGameSettingsOptions::GetMutableRow(EGameSettingsOption Option)
{
    return Rows[static_cast<int32>(Option)];
}
