// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Common/ReignForceActivatableWidget.h"
#include "GameSettingsWidget.generated.h"


class UButton;
class UTextBlock;
class UPanelWidget;
class UReignForceGameUserSettings;
class UKeyBindingsWidget;
class UGamepadSchemeWidget;

/**
 * Settings menu (video, audio & controls submenu) built natively; option rows change values with left/right navigation or a click
 */
UCLASS()
class REIGNFORCE_API UGameSettingsWidget : public UReignForceActivatableWidget
{
	GENERATED_BODY()

public:
	UGameSettingsWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Builds the menu taking buttons style from the template (should be called before adding to the viewport) */
	UFUNCTION(BlueprintCallable)
	void Setup(UButton* StyleTemplate);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual bool HandleBackAction() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Layout", meta = (ClampMin = 0, UIMin = 0))
	float MenuWidth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Layout")
	FLinearColor BackgroundColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Controls")
	TSubclassOf<UKeyBindingsWidget> KeyBindingsMenuClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Controls")
	TSubclassOf<UGamepadSchemeWidget> GamepadSchemeMenuClass;

private:
	/** The menu integration tests change options directly (a headless session has no keyboard focus for the rows) */
	friend class FSettingsMenuIntegrationTests;

	struct FOptionRow
	{
		FText Label;
		TArray<FText> Names;
		TArray<int32> Values;
		int32 Index = 0;
		/** The value the settings have now (read or applied last), Apply is enabled only when some row differs from it */
		int32 AppliedIndex = 0;
		bool bLive = false;
		TWeakObjectPtr<UButton> Button;
		TWeakObjectPtr<UTextBlock> ValueText;
		TWeakObjectPtr<UTextBlock> PreviousArrow;
		TWeakObjectPtr<UTextBlock> NextArrow;

		int32 GetValue() const { return Values.IsValidIndex(Index) ? Values[Index] : 0; }
	};

	enum EOption : int32
	{
		WindowMode, Resolution, Quality, VSync, FrameRate, LookSensitivity, MasterVolume, MusicVolume, EffectsVolume,
		InvertMouseX, InvertMouseY, InvertGamepadX, InvertGamepadY, Count
	};

	/** Look axis inversion rows: the device (mouse or gamepad stick) & the axis they flip */
	struct FInversionOption
	{
		EOption Option;
		bool bGamepad;
		bool bVertical;
	};
	static TConstArrayView<FInversionOption> GetInversionOptions();

	TArray<FOptionRow> Rows;
	TArray<FIntPoint> Resolutions;
	bool bBuilt;

	UPROPERTY()
	TObjectPtr<UButton> ApplyButton;

	UPROPERTY()
	TObjectPtr<UTextBlock> ApplyLabel;

	UPROPERTY()
	TObjectPtr<UButton> BackButton;

	UPROPERTY()
	TObjectPtr<UButton> ControlsButton;

	UPROPERTY()
	TObjectPtr<UButton> GamepadSchemeButton;

	/** Submenus take the same buttons style */
	TWeakObjectPtr<UButton> ButtonsStyleTemplate;

	UFUNCTION()
	UWidget* HandleOptionNavigation(EUINavigation Direction);

	UFUNCTION()
	void HandleOptionClicked();

	UFUNCTION()
	void HandleApplyClicked();

	UFUNCTION()
	void HandleControlsClicked();

	UFUNCTION()
	void HandleGamepadSchemeClicked();

	UFUNCTION()
	void HandleBackClicked();

	void ReadOptions(const UReignForceGameUserSettings* Settings);
	void ChangeOption(int32 RowIndex, int32 Delta, bool bWrap);
	void PreviewOption(int32 RowIndex);
	void RefreshRowText(int32 RowIndex);
	int32 FindActiveRow(bool bPreferHovered) const;
	void DiscardChanges();

	bool HasPendingChanges() const;
	/** Enables Apply only with pending changes & keeps the gamepad / keyboard navigation off it while it's disabled */
	void RefreshApplyState();
};
