// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Common/ReignForceActivatableWidget.h"
#include "Settings/GameSettingsOption.h"
#include "Settings/GameSettingsOptions.h"
#include "GameSettingsWidget.generated.h"


class UButton;
class UTextBlock;
class UVerticalBox;
class UKeyBindingsWidget;
class UGamepadSchemeWidget;

/**
 * Settings menu (video, audio & controls submenus) built natively; option rows change values with left/right navigation or a click
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

	/** Steps the option value like left / right on its row (bWrap - like a click: the last value goes on to the first one) */
	UFUNCTION(BlueprintCallable)
	void ChangeOption(EGameSettingsOption Option, int32 Delta, bool bWrap);

	/** Applies & saves the changed options, the menu stays open with them as the new baseline */
	UFUNCTION(BlueprintCallable)
	void ApplyChanges();

	/** Restores the live previewed options (volumes) from the saved settings */
	UFUNCTION(BlueprintCallable)
	void DiscardChanges();

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool HasPendingChanges() const { return Options.HasPendingChanges(); }

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
	/** Widgets showing an option row */
	struct FOptionRowView
	{
		TWeakObjectPtr<UButton> Button;
		TWeakObjectPtr<UTextBlock> ValueText;
		TWeakObjectPtr<UTextBlock> PreviousArrow;
		TWeakObjectPtr<UTextBlock> NextArrow;
	};

	FGameSettingsOptions Options;
	TArray<FOptionRowView> RowViews;
	bool bBuilt;

	UPROPERTY()
	TObjectPtr<UButton> ApplyButton;

	UPROPERTY()
	TObjectPtr<UTextBlock> ApplyLabel;

	UPROPERTY()
	TObjectPtr<UButton> BackButton;

	UPROPERTY()
	TObjectPtr<UButton> GamepadSchemeButton;

	/** Submenus take the same buttons style */
	TWeakObjectPtr<UButton> ButtonsStyleTemplate;

	UFUNCTION()
	UWidget* HandleOptionNavigation(EUINavigation Direction);

	UFUNCTION()
	void HandleOptionClicked();

	UFUNCTION()
	void HandleControlsClicked();

	UFUNCTION()
	void HandleGamepadSchemeClicked();

	UFUNCTION()
	void HandleBackClicked();

	void AddOptionRow(UVerticalBox* List, EGameSettingsOption Option, const FSlateFontInfo& Font);
	UButton* AddSubmenuButton(UVerticalBox* List, const TCHAR* Label, FName Name, const FSlateFontInfo& Font);
	void AddActions(UVerticalBox* Column, const FSlateFontInfo& Font);
	void RefreshRowText(EGameSettingsOption Option);
	/** Option of the row a click (hovered first) or left / right (focused) is for, COUNT if none */
	EGameSettingsOption FindActiveOption(bool bPreferHovered) const;

	/** Enables Apply only with pending changes & keeps the gamepad / keyboard navigation off it while it's disabled */
	void RefreshApplyState();
};
