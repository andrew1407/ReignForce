// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Framework/Application/IInputProcessor.h"


DECLARE_DELEGATE_OneParam(FOnKeyCaptured, const FKey&);

/**
 * Takes the next pressed key (keyboard, mouse button or wheel) before any widget or the game input gets it and reports it
 * while it's bound (gamepad sticks are swallowed too: they would move the focus while a key is awaited)
 */
class REIGNFORCE_API FKeyCaptureInputProcessor : public IInputProcessor
{
public:
	explicit FKeyCaptureInputProcessor(FOnKeyCaptured InOnKeyCaptured) : OnKeyCaptured(MoveTemp(InOnKeyCaptured)) {}

	/** Stops reporting the keys (Slate keeps an unregistered processor alive until the current event is processed) */
	FORCEINLINE void Detach() { OnKeyCaptured.Unbind(); }

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {}
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override;
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override;
	virtual const TCHAR* GetDebugName() const override { return TEXT("KeyBindingsCapture"); }

private:
	FOnKeyCaptured OnKeyCaptured;

	/** Reports the key (repeats are swallowed only), returns true if the event is consumed */
	bool Capture(const FKey& Key, bool bRepeat = false);
};
