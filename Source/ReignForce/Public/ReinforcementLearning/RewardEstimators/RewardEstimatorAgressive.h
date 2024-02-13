// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ReinforcementLearning/RewardEstimators/RewardEstimator.h"
#include "RewardEstimatorAgressive.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API URewardEstimatorAgressive : public URewardEstimator
{
	GENERATED_BODY()

public:
    virtual float CalculateReward_Implementation(const FAIClientRequest& Data);
};
