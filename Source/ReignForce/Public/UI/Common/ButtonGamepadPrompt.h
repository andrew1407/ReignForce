// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Layout/Margin.h"
#include "ButtonGamepadPrompt.generated.h"


class UButton;
class UImage;
class USizeBox;
class UTextBlock;
class UWidgetTree;

/**
 * Gamepad button prompt in front of a button label ([prompt][label], centered together): it slides out from behind the label
 * while its width grows from zero, pops when its key changes and widens the button when the label & prompt don't fit
 */
USTRUCT()
struct FButtonGamepadPrompt
{
	GENERATED_USTRUCT_BODY()

public:
	/** Puts the (hidden) prompt in front of the button content: [prompt][content] */
	void Attach(UWidgetTree* WidgetTree, UButton* Button, const FMargin& StylePadding);

	/** Moves the prompt towards the given key (invalid - hidden) */
	void Update(UButton* Button, const FKey& Key, float DeltaTime);

	/** Prompt shown now, invalid if none (it's animated, so it may be still appearing) */
	FKey GetShownKey() const;

	FORCEINLINE bool IsAttached() const { return GamepadIcon.IsValid(); }

private:
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
	float Progress = 0;

	/** Short scale pulse when the prompt key changes while it's shown (B -> A on focus) */
	float Pulse = 0;

	float Size = 0;

	/** Button style padding around the content */
	FMargin StylePadding;

	/** Width the button had without the prompt, it grows from it when the label & prompt don't fit */
	float BaseWidth = 0;

	/** The base width is the fixed size canvas slot width (known before the layout, the slot may grow freely) */
	bool bBaseWidthFromSlot = false;

	/** Growth applied to the button slot now (slot layout is changed only when it differs) */
	float AppliedGrowth = 0;

	/** Canvas slot offsets or box slot paddings before the growth */
	FMargin BaseSlotLayout;

	void ChangeKey(const FKey& Key);
	/** Prompt size, slide & fade for its progress, the button grows if needed */
	void ApplyProgress(UButton* Button);
	void ApplyGrowth(UButton* Button, float Growth);
};
