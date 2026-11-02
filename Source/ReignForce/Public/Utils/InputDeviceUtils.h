// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InputDeviceUtils.generated.h"


class ULocalPlayer;

/**
 * Input device the player uses now (CommonUI input type: switched by the last used device)
 */
UCLASS()
class REIGNFORCE_API UInputDeviceUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Input")
	static bool IsGamepadUsed(const ULocalPlayer* Player);
};
