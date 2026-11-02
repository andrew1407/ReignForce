// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Styling/SlateTypes.h"
#include "InputCoreTypes.h"
#include "UI/Common/FocusOutline.h"
#include "UI/Common/ButtonGamepadPrompt.h"
#include "ReignForceActivatableWidget.generated.h"


class UButton;
class USizeBox;
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
	bool bHighlighted = false;

	UPROPERTY()
	FButtonGamepadPrompt Prompt;
};

/**
 * Common UI menu base: back action (Escape / gamepad B) handling, keyboard & gamepad focus keeping, a noticeable focused button
 * highlight with gamepad prompts and submenus stacking
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

	/** The button the back action (Escape / gamepad B) stands for: it shows the B prompt while it isn't focused */
	UFUNCTION(BlueprintCallable)
	void SetBackActionButton(UButton* Button);

	/**
	 * Gamepad prompt key of a registered button (invalid - no prompt) while the menu is active:
	 * the confirm button (A) on the focused button, the back button (B) on the back action button
	 */
	virtual FKey GetGamepadPromptKey(const UButton* Button, bool bFocused) const;

	/** Prompt shown on a registered button now, invalid if none (it's animated, so it may be still appearing) */
	FKey GetShownGamepadPrompt(const UButton* Button) const;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual void NativeDestruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
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

	/** The box height is limited to the screen height without the margins (top & bottom), the content scrolls beyond it */
	void LimitHeightToScreen(USizeBox* Box, float ScreenMargin);

	/** Items column (panel the given item is in, placed on a canvas) is centered vertically on the screen & scrolls when it doesn't fit */
	void MakeMenuItemsScrollableCentered(UWidget* Item, float ScreenMargin);

	bool IsButtonAvailable(const UButton* Button) const;
	bool IsGamepadUsed() const;

	/** Frame around the focused button (the button keeps its own look & size, so the layout and navigation aren't affected) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus")
	FFocusOutlineStyle FocusOutlineStyle;

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

	TWeakObjectPtr<UButton> ChildMenuOpener;

	TWeakObjectPtr<UButton> BackActionButton;

	UPROPERTY()
	TObjectPtr<USizeBox> ScreenHeightLimitedBox;

	float ScreenHeightMargin = 0;

	/** Deactivated menu isn't hit-testable (so not navigable) until it is activated again */
	ESlateVisibility VisibilityBeforeDeactivation = ESlateVisibility::SelfHitTestInvisible;
	bool bNavigationBlocked = false;

	/** Cursor position of the last focus change by the mouse hover */
	FVector2D HoverCursorPosition = FVector2D::ZeroVector;

	UFUNCTION()
	void HandleButtonHovered();

	/** Hover moves the focus only when the mouse itself moved (not the menu appearing under a resting cursor or the gamepad) */
	bool IsHoverFromMouseMove();

	void UpdateButtonsHighlight(float DeltaTime);
	void UpdateScreenHeightLimit(const FGeometry& MyGeometry);
	void RestoreLostFocus();
};
