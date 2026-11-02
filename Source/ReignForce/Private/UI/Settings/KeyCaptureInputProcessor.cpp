// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Settings/KeyCaptureInputProcessor.h"
#include "Input/Events.h"


bool FKeyCaptureInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
    return Capture(InKeyEvent.GetKey(), InKeyEvent.IsRepeat());
}

bool FKeyCaptureInputProcessor::HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent)
{
    return OnKeyCaptured.IsBound();
}

bool FKeyCaptureInputProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    return Capture(MouseEvent.GetEffectingButton());
}

bool FKeyCaptureInputProcessor::HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    return Capture(MouseEvent.GetEffectingButton());
}

bool FKeyCaptureInputProcessor::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent)
{
    const float Delta = InWheelEvent.GetWheelDelta();
    if (InGestureEvent || FMath::IsNearlyZero(Delta)) return OnKeyCaptured.IsBound();
    return Capture(Delta > 0 ? EKeys::MouseScrollUp : EKeys::MouseScrollDown);
}

bool FKeyCaptureInputProcessor::Capture(const FKey& Key, bool bRepeat)
{
    if (!OnKeyCaptured.IsBound()) return false;
    // The owner unregisters the processor on the capture: the delegate is copied, so it isn't destroyed while it runs
    if (!bRepeat) FOnKeyCaptured(OnKeyCaptured).ExecuteIfBound(Key);
    return true;
}
