// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AICommandsStrategy.generated.h"


// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAICommandsStrategy : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class REIGNFORCE_API IAICommandsStrategy
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AICommandsStrategy")
	bool RequestCommand(APawn* Executor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AICommandsStrategy")
	float GetRequestFrequency();
};
