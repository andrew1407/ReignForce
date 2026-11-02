// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/InputDeviceUtils.h"
#include "Engine/LocalPlayer.h"
#include "CommonInputSubsystem.h"


bool UInputDeviceUtils::IsGamepadUsed(const ULocalPlayer* Player)
{
    const UCommonInputSubsystem* InputSubsystem = IsValid(Player) ? UCommonInputSubsystem::Get(Player) : nullptr;
    return IsValid(InputSubsystem) && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;
}
