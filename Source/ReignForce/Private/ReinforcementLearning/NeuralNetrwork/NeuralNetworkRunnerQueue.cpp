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

	bool bAdded = InputsQueue.Enqueue({ MoveTemp(Inputs), MoveTemp(OnONeuralModelResult), Querier });
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

	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, QueueOptions = MoveTemp(QueueOptions)]
	{
        auto OnFailure = [this, &QueueOptions] { QueueOptions.OnResult.ExecuteIfBound({}, this, QueueOptions.Querier, false); };

		if (!IsModelValid())
		{
			bModelRunning = false;
			return OnFailure();
		}
	
		TArray<UE::NNE::FTensorBindingCPU> InputBindings;
		TArray<UE::NNE::FTensorBindingCPU> OutputBindings;

		TArray<float> InputsCopy = QueueOptions.Inputs;
		TArray<float> Outputs;

		bool IsParamsSet = UAICommandsUtils::SetNeuralModelParams(ModelInstance.Get(), InputBindings, OutputBindings, InputsCopy, Outputs);
		if (!IsParamsSet)
		{
			bModelRunning = false;
			return OnFailure();
		}

		int32 Status = ModelInstance->RunSync(InputBindings, OutputBindings);
		AsyncTask(ENamedThreads::GameThread, [this, Status, QueueOptions, Outputs = MoveTemp(Outputs)]
		{
			bool bSuccess = Status == 0;
			QueueOptions.OnResult.ExecuteIfBound(Outputs, this, QueueOptions.Querier, bSuccess);

			bModelRunning = false;
			HandleNextInQueue();
		});
	});

    return true;
}
