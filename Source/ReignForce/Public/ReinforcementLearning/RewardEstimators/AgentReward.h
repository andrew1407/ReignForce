#pragma once

#include "Containers/Set.h"
#include "AgentReward.generated.h"


USTRUCT(BlueprintType)
struct FAgentReward
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(BlueprintReadWrite)
    float Reward = 0;

    void Add(float Value = 1) { Reward += Value; }
    void Substract(float Value = 1) { Reward -= Value; }

    float operator()() const { return Reward; }
    float& operator*() { return Reward; }
};
