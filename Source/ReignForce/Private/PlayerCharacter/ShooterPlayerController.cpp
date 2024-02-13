// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter/ShooterPlayerController.h"


void AShooterPlayerController::SetInputModeUI()
{
    SetInputMode(FInputModeUIOnly());
    SetShowMouseCursor(true);
}

void AShooterPlayerController::SetInputModeGameplay()
{
    SetInputMode(FInputModeGameOnly());
    SetShowMouseCursor(false);
}

bool AShooterPlayerController::SetInteractionMode(EPlayerInteractionMode Mode)
{
    bool bSet = false;

    if (Mode == EPlayerInteractionMode::Pause)
    {
        bSet = SetPause(true);
    }
    else if (Mode == EPlayerInteractionMode::UIInputAndPause)
    {
        SetInputModeUI();
        bSet = SetPause(true);
    }
    else if (Mode == EPlayerInteractionMode::GameplayInput)
    {
        SetInputModeGameplay();
        bSet = true;
    }
    else if (Mode == EPlayerInteractionMode::UIInput)
    {
        SetInputModeUI();
        bSet = true;
    }
    else if (Mode == EPlayerInteractionMode::NotPause)
    {
        bSet = SetPause(false);
    }
    else if (Mode == EPlayerInteractionMode::GameplayInputAndNotPause)
    {
        SetInputModeGameplay();
        bSet = SetPause(false);
    }

    return bSet;
}
