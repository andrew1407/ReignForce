// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Common/ReignForceActivatableWidget.h"
#include "KeyBindingsWidget.generated.h"


class UButton;
class UTextBlock;
class UVerticalBox;
class IInputProcessor;
class UEnhancedInputUserSettings;
struct FPlayerKeyMapping;

/**
 * Keyboard & mouse key bindings menu built natively: a row click waits for the new key (Escape / gamepad B cancels),
 * a taken key is swapped with the other row, each row can be reset to its default key. Changes are applied & saved right away
 */
UCLASS()
class REIGNFORCE_API UKeyBindingsWidget : public UReignForceActivatableWidget
{
	GENERATED_BODY()

	friend class FKeyCaptureInputProcessor;

public:
	UKeyBindingsWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Builds the menu taking buttons style from the template (should be called before adding to the viewport) */
	UFUNCTION(BlueprintCallable)
	void Setup(UButton* StyleTemplate);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;
	virtual bool HandleBackAction() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Layout", meta = (ClampMin = 0, UIMin = 0))
	float MenuWidth;

	/** Bindings list is scrolled within this height */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Layout", meta = (ClampMin = 0, UIMin = 0))
	float ListHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Layout")
	FLinearColor BackgroundColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Layout")
	FLinearColor CategoryColor;

	/** Key text color of the rows bound to the same key */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Layout")
	FLinearColor ConflictColor;

	/** Key text color of the row waiting for a key */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Layout")
	FLinearColor CaptureColor;

private:
	struct FBindingRow
	{
		FName MappingName;
		FText Label;
		TWeakObjectPtr<UButton> Button;
		TWeakObjectPtr<UButton> ResetButton;
		TWeakObjectPtr<UTextBlock> KeyText;
		TWeakObjectPtr<UTextBlock> ResetText;
	};

	TArray<FBindingRow> Rows;
	int32 CapturingRow;
	bool bBuilt;

	TSharedPtr<IInputProcessor> KeyCapture;

	UPROPERTY()
	TObjectPtr<UButton> ResetAllButton;

	UPROPERTY()
	TObjectPtr<UButton> BackButton;

	UFUNCTION()
	void HandleBindingClicked();

	UFUNCTION()
	void HandleResetClicked();

	UFUNCTION()
	void HandleResetAllClicked();

	UEnhancedInputUserSettings* GetInputSettings() const;
	const FPlayerKeyMapping* FindKeyboardMapping(const UEnhancedInputUserSettings* Settings, FName MappingName) const;

	/** Remappable keyboard & mouse mappings of the gameplay mapping context in its order, grouped by categories */
	TArray<TPair<FText, TArray<FBindingRow>>> ReadBindings(UEnhancedInputUserSettings* Settings) const;

	void StartCapture(int32 RowIndex);
	void StopCapture();
	void HandleCapturedKey(const FKey& Key);
	void BindKey(int32 RowIndex, const FKey& Key);
	void SaveBindings(UEnhancedInputUserSettings* Settings);
	void RefreshRows();
	int32 FindActiveRow(bool bResetButton) const;
	void PlayClickSound() const;
};
