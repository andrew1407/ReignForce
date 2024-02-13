// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ReinforcementLearning/RewardEstimators/RewardEstimator.h"
#include "RewardEstimatorRetreat.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API URewardEstimatorRetreat : public URewardEstimator
{
	GENERATED_BODY()

public:
    virtual float CalculateReward_Implementation(const FAIClientRequest& Data);
};
