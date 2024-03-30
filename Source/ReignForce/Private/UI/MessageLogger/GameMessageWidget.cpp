// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MessageLogger/GameMessageWidget.h"
#include "Components/TextBlock.h"


void UGameMessageWidget::SetMessageText(const FText& ToSet)
{
    if (IsValid(Message)) Message->SetText(ToSet);
}

void UGameMessageWidget::NativeDestruct()
{
    CancelDestroyTimeout();

    Super::NativeDestruct();
}

bool UGameMessageWidget::SetDestroyTimeout(float Seconds)
{
    bool bEnvValid = IsValid(this) && IsValid(GetWorld());
    if (!bEnvValid) return false;

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    if (CleanupTimer.IsValid() || TimerManager.IsTimerActive(CleanupTimer)) return false;

    TimerManager.SetTimer(CleanupTimer, [this]
    {
        bool bEnvValid = IsValid(this) && IsValid(GetWorld());
        if (bEnvValid) RemoveFromParent();
    }, Seconds, false);

    return true;
}

bool UGameMessageWidget::CancelDestroyTimeout()
{
    bool bEnvValid = IsValid(this) && IsValid(GetWorld());
    if (!bEnvValid) return false;

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();
    if (!(CleanupTimer.IsValid() && TimerManager.IsTimerActive(CleanupTimer))) return false;
    TimerManager.ClearTimer(CleanupTimer);
    return true;
}
