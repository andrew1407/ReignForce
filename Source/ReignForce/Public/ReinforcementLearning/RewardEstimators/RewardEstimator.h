// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ReinforcementLearning/Connection/AIClientRequest.h"
#include "RewardEstimator.generated.h"


/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class REIGNFORCE_API URewardEstimator : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "RewardEstimator")
    float CalculateReward(const FAIClientRequest& Data);

    virtual float CalculateReward_Implementation(const FAIClientRequest& Data) { return 0; }
};
