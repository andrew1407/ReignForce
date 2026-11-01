// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Styling/SlateTypes.h"
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

	/** Gamepad confirm button icon in front of the label, shown on the focused button while a gamepad is used */
	UPROPERTY()
	TWeakObjectPtr<UImage> GamepadIcon;
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

	void UpdateButtonsHighlight();
	bool IsButtonAvailable(const UButton* Button) const;

	/** Puts a (hidden) gamepad confirm icon in front of the button content: [icon][label] */
	UImage* AddGamepadIcon(UButton* Button);
	bool IsGamepadUsed() const;
};
