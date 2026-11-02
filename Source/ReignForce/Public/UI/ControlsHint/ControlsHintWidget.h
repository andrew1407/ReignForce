// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "ControlsHintWidget.generated.h"


class UBorder;
class UGridPanel;
class UInputMappingContext;

/** A row of the controls hint: the action label & the input action (by its asset name) the keys are taken from */
USTRUCT(BlueprintType)
struct FControlsHintEntry
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ActionName;

	/** Keys of the action shown at most (e.g. W A S D for the movement) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 1, UIMin = 1))
	int32 MaxKeys = 1;
};

UENUM(BlueprintType)
enum class EControlsHintPhase : uint8
{
	WAITING UMETA(DisplayName="Waiting"),
	SLIDING_IN UMETA(DisplayName="Sliding in"),
	SHOWN UMETA(DisplayName="Shown"),
	SLIDING_OUT UMETA(DisplayName="Sliding out"),
	FINISHED UMETA(DisplayName="Finished"),
};

/**
 * Main controls table shown for a few seconds when the game level is opened (top left corner over the HUD, slides in from & out to the left side):
 * keyboard & mouse keys (the player's bindings) or gamepad button icons, switched with the used input device
 */
UCLASS()
class REIGNFORCE_API UControlsHintWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UControlsHintWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Shows the gamepad (true) or keyboard & mouse keys */
	UFUNCTION(BlueprintCallable)
	void ShowInputDevice(bool bGamepad);

	/** Moves the slide in, show & slide out timeline and the input device switch on (called every tick) */
	void Advance(float DeltaTime);

	UFUNCTION(BlueprintPure)
	FORCEINLINE EControlsHintPhase GetPhase() const { return Phase; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsShowingGamepad() const { return bShowingGamepad; }

	/** Keys of the action (by its asset name) from the remappable controls: keyboard & mouse ones with the player's bindings, or the gamepad ones */
	static TArray<FKey> FindActionKeys(const UInputMappingContext* Context, FName ActionName, bool bGamepad, int32 MaxKeys, const ULocalPlayer* Player = nullptr);

	/** Compact key name for a key cap: LMB, Esc, Space, W */
	static FText GetKeyCapText(const FKey& Key);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
	TArray<FControlsHintEntry> Entries;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Delay after the level opening before the table slides in (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = 0, UIMin = 0, Units = "Seconds"))
	float StartDelay;

	/** How long the table is fully shown (game time: it waits while the game is paused) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = 0, UIMin = 0, Units = "Seconds"))
	float ShowDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = .01, UIMin = .01, Units = "Seconds"))
	float SlideDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = .01, UIMin = .01, Units = "Seconds"))
	float DeviceSwitchFadeDuration;

	/** The panel is as wide as its labels & keys, at least this (keeps it steady when the device switches to shorter keys) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout")
	float PanelMinWidth;

	/** Distance from the screen top left corner */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout")
	FVector2D ScreenOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout")
	FLinearColor PanelColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout")
	FLinearColor OutlineColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout")
	FLinearColor TitleColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout")
	FLinearColor KeyCapColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Layout")
	FLinearColor KeyCapOutlineColor;

private:
	UPROPERTY()
	TObjectPtr<UBorder> Panel;

	UPROPERTY()
	TObjectPtr<UGridPanel> Table;

	EControlsHintPhase Phase = EControlsHintPhase::WAITING;
	float PhaseTime = 0;
	bool bBuilt = false;
	bool bShowingGamepad = false;
	/** Device the table switches to after fading out (the table content is rebuilt while it's invisible) */
	bool bPendingGamepad = false;
	bool bDeviceSwitchPending = false;
	float TableOpacity = 1;

	void Build();
	void FillTable(bool bGamepad);
	UWidget* MakeKeyCap(const FKey& Key, const FSlateFontInfo& Font);
	UWidget* MakeGamepadIcon(const FKey& Key, const FSlateFontInfo& Font);
	void ApplySlide(float Shown);
	void AdvanceDeviceSwitch(float DeltaTime);
	void AdvancePhase(float DeltaTime);
	bool IsGamePaused() const;
};
