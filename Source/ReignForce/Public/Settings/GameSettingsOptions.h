// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Settings/GameSettingsOption.h"


class UReignForceGameUserSettings;

/** Option row: the values it steps through with their names, the selected one & the one the settings have now */
struct REIGNFORCE_API FGameSettingsOptionRow
{
    FText Label;
    TArray<FText> Names;
    TArray<int32> Values;
    int32 Index = 0;
    /** The value the settings have now (read or applied last), there is something to apply only when some row differs from it */
    int32 AppliedIndex = 0;
    /** Previewed right away, before it's applied (volumes) */
    bool bLive = false;

    static FGameSettingsOptionRow Of(const FText& Label, TArray<FText> Names, TArray<int32> Values, bool bLive = false);

    FORCEINLINE int32 GetValue() const { return Values.IsValidIndex(Index) ? Values[Index] : 0; }
    FORCEINLINE bool HasPrevious() const { return Index > 0; }
    FORCEINLINE bool HasNext() const { return Index < Names.Num() - 1; }
    FORCEINLINE bool IsChanged() const { return Index != AppliedIndex; }

    /** Selects the value closest to the given one */
    void Select(int32 Value);
};

/**
 * Settings menu model (framework-free of the UI): option rows read from the user settings, stepped & written back on apply
 */
class REIGNFORCE_API FGameSettingsOptions
{
public:
    /** Rows with the current values of the settings selected (the defaults without the settings) */
    static FGameSettingsOptions Of(const UReignForceGameUserSettings* Settings);

    /** Steps the option value (wrapping around or stopping at the ends), returns false if it stays the same */
    bool Change(EGameSettingsOption Option, int32 Delta, bool bWrap);

    /** Writes the selected values into the settings (not applied to the engine nor saved yet) */
    void WriteTo(UReignForceGameUserSettings* Settings) const;

    /** Writes a live option (volume) into the settings for a preview */
    void PreviewTo(UReignForceGameUserSettings* Settings, EGameSettingsOption Option) const;

    /** The selected values became the settings ones */
    void MarkApplied();

    bool HasPendingChanges() const;

    const FGameSettingsOptionRow& GetRow(EGameSettingsOption Option) const;

private:
    TArray<FGameSettingsOptionRow> Rows;
    TArray<FIntPoint> Resolutions;

    FGameSettingsOptionRow& GetMutableRow(EGameSettingsOption Option);
    void ReadResolutions(const FIntPoint& Current);
    void SelectCurrentValues(const UReignForceGameUserSettings* Settings);
};
