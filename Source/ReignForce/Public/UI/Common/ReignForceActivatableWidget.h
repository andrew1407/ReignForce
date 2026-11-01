// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Styling/SlateTypes.h"
#include "InputCoreTypes.h"
#include "UI/Common/FocusOutline.h"
#include "ReignForceActivatableWidget.generated.h"


class UButton;
class UImage;
class UTextBlock;
class UVerticalBox;
class USizeBox;
class UScrollBox;
class UGameSettingsWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuCloseRequested);

/** Space around the scrolled menu content (focus frames stay inside the scroll box clipping) */
constexpr float SCROLLED_CONTENT_INSET = 8;

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

	/** Gamepad button prompt in front of the label ([prompt][label], centered together): its width grows from zero while it shows up */
	UPROPERTY()
	TWeakObjectPtr<USizeBox> PromptBox;

	UPROPERTY()
	TWeakObjectPtr<UImage> GamepadIcon;

	/** The label the button needs room for (centered labels only) */
	UPROPERTY()
	TWeakObjectPtr<UTextBlock> CenteredLabel;

	/** Shown prompt key (kept while the prompt hides) */
	UPROPERTY()
	FKey PromptKey;

	/** 0 - hidden, 1 - fully shown */
	float PromptProgress = 0;

	/** Short scale pulse when the prompt key changes while it's shown (B -> A on focus) */
	float PromptPulse = 0;

	float PromptSize = 0;

	/** Width the button had without the prompt, it grows from it when the label & prompt don't fit */
	float BaseWidth = 0;

	/** The base width is the fixed size canvas slot width (known before the layout, the slot may grow freely) */
	bool bBaseWidthFromSlot = false;

	/** Growth applied to the button slot now (slot layout is changed only when it differs) */
	float AppliedGrowth = 0;

	/** Canvas slot offsets or box slot paddings before the growth */
	FMargin BaseSlotLayout;
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

	/** The button the back action (Escape / gamepad B) stands for: it shows the B prompt while it isn't focused */
	UFUNCTION(BlueprintCallable)
	void SetBackActionButton(UButton* Button);

	/**
	 * Gamepad prompt key of a registered button (invalid - no prompt) while the gamepad is used & the menu is active:
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

	/** Clones the template button style & label into a new button placed next to the template (same panel) */
	UButton* CreateButtonLike(UButton* Template, const FText& Label, FName Name, int32 IndexOffset = 0);

	/** Places the items of the panel the given item is in one after another with the given gap (spacers or paddings) */
	void SetMenuItemsGap(UWidget* Item, float Gap);

	/** Puts the content into a vertical scroll box which takes the content place (slot) inside a size box (returned) */
	USizeBox* WrapInScrollBox(UWidget* Content, UScrollBox** OutScrollBox = nullptr);

	/** The box height is limited to the screen height without the margins (top & bottom), the content scrolls beyond it */
	void LimitHeightToScreen(USizeBox* Box, float ScreenMargin);

	/** Items column (panel the given item is in, placed on a canvas) is centered vertically on the screen & scrolls when it doesn't fit */
	void MakeMenuItemsScrollableCentered(UWidget* Item, float ScreenMargin);

	#pragma region NATIVE_LAYOUT

	/** Template button label font of the given size (engine default bold font without a template) */
	static FSlateFontInfo MakeFont(const UButton* StyleTemplate, int32 Size);

	/** Button with the template style (brushes, sounds, colors) holding the content */
	UButton* MakeButton(UButton* StyleTemplate, UWidget* Content, FName Name);

	UTextBlock* MakeText(const FText& Text, const FSlateFontInfo& Font, const FSlateColor& Color);

	/** Dimmed fullscreen background with a centered titled column (menu content is added to it) */
	UVerticalBox* ConstructMenuColumn(const FText& Title, const FSlateFontInfo& TitleFont, float Width, const FLinearColor& Background);

	#pragma endregion

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
	bool IsButtonAvailable(const UButton* Button) const;

	/** Puts a (hidden) gamepad prompt in front of the button content: [prompt][content] */
	void AddGamepadPrompt(FFocusableButtonState& State);
	/** Prompt size, slide & fade for its progress, the button grows if needed */
	void ApplyPromptProgress(FFocusableButtonState& State);
	void ApplyButtonGrowth(FFocusableButtonState& State, float Growth);

protected:
	bool IsGamepadUsed() const;
};
