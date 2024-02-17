// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Rounds/RestartLevelAfterLoseWidget.h"
#include "Components/TextBlock.h"


URestartLevelAfterLoseWidget::URestartLevelAfterLoseWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    CountdownSecondsToRestart = 3;
    CountdownSecondsLeft = CountdownSecondsToRestart;
}

FText URestartLevelAfterLoseWidget::GetCountdownDescriptionMessage() const
{
    return FText::FromString(FString::Printf(TEXT("Level will be restarted in %d seconds."), CountdownSecondsLeft));
}

void URestartLevelAfterLoseWidget::ResetCountdownMessage()
{
    if (IsValid(RestartMessage)) RestartMessage->SetText(GetCountdownDescriptionMessage());
}

bool URestartLevelAfterLoseWidget::DecreaseCurrentSecondsCountdown(bool bUpdateWidget)
{
    if (CountdownSecondsLeft <= 0) return false;
    --CountdownSecondsLeft;
    if (bUpdateWidget) ResetCountdownMessage();
    return true;
}
