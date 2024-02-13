// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ReinforcementLearning/NeuralNetrwork/NeuralNetworkRunner.h"
#include "NeuralNetworkRunnerQueue.generated.h"


struct FNNQueueOptions
{
    TArray<float> Inputs;
	FOnONeuralModelResult OnResult;
	UObject* Querier = nullptr;
};

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UNeuralNetworkRunnerQueue : public UNeuralNetworkRunner
{
	GENERATED_BODY()

public:
	UNeuralNetworkRunnerQueue(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Queue", meta = (ClampMin = 0, UIMin = 0))
	int32 QueueSizeLimit;

	virtual bool SendInputs(TArray<float> Inputs, FOnONeuralModelResult OnONeuralModelResult, UObject* Querier = nullptr) override;

	FORCEINLINE int32 GetTasksInQueueCount() const { return TasksInQueue; }
	FORCEINLINE bool IsModelRunning() const { return bModelRunning; }

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bModelRunning;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	int32 TasksInQueue;

	TQueue<FNNQueueOptions> InputsQueue;

	bool HandleNextInQueue();
};
