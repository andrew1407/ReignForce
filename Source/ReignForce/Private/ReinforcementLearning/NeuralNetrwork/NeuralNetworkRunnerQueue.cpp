// Fill out your copyright notice in the Description page of Project Settings.


#include "ReinforcementLearning/NeuralNetrwork/NeuralNetworkRunnerQueue.h"
#include "NNE.h"
#include "NNEModelData.h"

#include "ReinforcementLearning/AICommandsUtils.h"


UNeuralNetworkRunnerQueue::UNeuralNetworkRunnerQueue(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    QueueSizeLimit = 0;
    TasksInQueue = 0;
    bModelRunning = false;
}

bool UNeuralNetworkRunnerQueue::SendInputs(TArray<float> Inputs, FOnONeuralModelResult OnONeuralModelResult, UObject* Querier)
{
    if (!IsModelValid()) return false;

    bool bWasEmpty = InputsQueue.IsEmpty();
    if (QueueSizeLimit > 0 && TasksInQueue > QueueSizeLimit) return false;

	bool bAdded = InputsQueue.Enqueue({ MoveTemp(Inputs), MoveTemp(OnONeuralModelResult), Querier, Querier != nullptr });
	if (!bAdded) return false;

    ++TasksInQueue;
    bool bCanHandle = bWasEmpty && !IsModelRunning();
	return bCanHandle && HandleNextInQueue();
}

bool UNeuralNetworkRunnerQueue::HandleNextInQueue()
{
    if (InputsQueue.IsEmpty() || IsModelRunning() || !IsModelValid()) return false;

	bModelRunning = true;
	FNNQueueOptions QueueOptions;
	if (!InputsQueue.Dequeue(QueueOptions))
	{
		bModelRunning = false;
		return false;
	}
    --TasksInQueue;

	// Weak self: the queue may be collected (level change) while the model is running, the model instance is kept by the task
	RunModelAsync(ModelInstance, MoveTemp(QueueOptions.Inputs), [WeakSelf = TWeakObjectPtr<UNeuralNetworkRunnerQueue>(this), OnResult = MoveTemp(QueueOptions.OnResult),
		Querier = QueueOptions.Querier, bHasQuerier = QueueOptions.bHasQuerier] (TArray<float> Outputs, bool bSuccess)
	{
		UNeuralNetworkRunnerQueue* Self = WeakSelf.Get();
		if (!Self) return;
		UObject* QuerierObject = Querier.Get();
		if (!bHasQuerier || QuerierObject) OnResult.ExecuteIfBound(Outputs, Self, QuerierObject, bSuccess);

		Self->bModelRunning = false;
		Self->HandleNextInQueue();
	});

    return true;
}
