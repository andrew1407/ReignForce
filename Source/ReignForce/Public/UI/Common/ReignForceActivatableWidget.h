// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Styling/SlateTypes.h"
#include "ReignForceActivatableWidget.generated.h"


class UButton;
class UTextBlock;
class UVerticalBox;
class UGameSettingsWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuCloseRequested);

USTRUCT()
struct FFocusableButtonState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TWeakObjectPtr<UButton> Button;

	UPROPERTY()
	FButtonStyle DefaultStyle;

	UPROPERTY()
	FVector2D DefaultScale = FVector2D(1);

	UPROPERTY()
	bool bHighlighted = false;
};

/**
 * Common UI menu base: back action (Escape / gamepad B) handling, keyboard & gamepad focus keeping and a noticeable focused button highlight
 */
UCLASS(Abstract)
class REIGNFORCE_API UReignForceActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UReignForceActivatableWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(BlueprintAssignable)
	FOnMenuCloseRequested OnCloseRequested;

	UFUNCTION(BlueprintCallable)
	void RegisterFocusableButton(UButton* Button);

	UFUNCTION(BlueprintCallable)
	void FocusButton(UButton* Button);

	UFUNCTION(BlueprintCallable)
	void RequestClose();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual bool NativeOnHandleBackAction() override;

	/** Back action handler, returns true if the action is consumed */
	virtual bool HandleBackAction();

	/** Shows the menu over this one, which is deactivated until the child requests closing */
	void PushChildMenu(UReignForceActivatableWidget* Child, int32 ZOrder = 10);

	UFUNCTION(BlueprintCallable)
	void CloseChildMenu();

	UFUNCTION(BlueprintCallable)
	void OpenSettingsMenu(UButton* StyleTemplate);

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsChildMenuOpened() const { return IsValid(ChildMenu); }

	/** Clones the template button style & label into a new button placed next to the template (same panel) */
	UButton* CreateButtonLike(UButton* Template, const FText& Label, FName Name, int32 IndexOffset = 0);

	#pragma region NATIVE_LAYOUT

	/** Template button label font of the given size (engine default bold font without a template) */
	static FSlateFontInfo MakeFont(const UButton* StyleTemplate, int32 Size);

	/** Button with the template style (brushes, sounds, colors) holding the content */
	UButton* MakeButton(UButton* StyleTemplate, UWidget* Content, FName Name);

	UTextBlock* MakeText(const FText& Text, const FSlateFontInfo& Font, const FSlateColor& Color);

	/** Dimmed fullscreen background with a centered titled column (menu content is added to it) */
	UVerticalBox* ConstructMenuColumn(const FText& Title, const FSlateFontInfo& TitleFont, float Width, const FLinearColor& Background);

	#pragma endregion

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus", meta = (ClampMin = 1, UIMin = 1))
	float FocusedButtonScale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus")
	FLinearColor FocusedButtonTint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus")
	bool bRestoreLostFocus;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu|Settings")
	TSubclassOf<UGameSettingsWidget> SettingsMenuClass;

private:
	UPROPERTY()
	TArray<FFocusableButtonState> FocusableButtons;

	UPROPERTY()
	TObjectPtr<UReignForceActivatableWidget> ChildMenu;

	TWeakObjectPtr<UButton> LastFocusedButton;

	UFUNCTION()
	void HandleButtonHovered();

	void UpdateButtonsHighlight();
	void SetButtonHighlight(FFocusableButtonState& State, bool bHighlight);
	bool IsButtonAvailable(const UButton* Button) const;
};
