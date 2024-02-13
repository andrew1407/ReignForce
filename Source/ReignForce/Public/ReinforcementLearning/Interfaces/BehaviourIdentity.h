// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ReinforcementLearning/TemperamentType.h"
#include "BehaviourIdentity.generated.h"


// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBehaviourIdentity : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class REIGNFORCE_API IBehaviourIdentity
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BehaviourIdentity")
	ETemperamentType GetTemperamentType();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BehaviourIdentity")
	void ChangeTemperamentType(ETemperamentType TemperamentToSet);
};
