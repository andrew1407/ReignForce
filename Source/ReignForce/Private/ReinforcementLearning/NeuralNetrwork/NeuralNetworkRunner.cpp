// Fill out your copyright notice in the Description page of Project Settings.


#include "ReinforcementLearning/NeuralNetrwork/NeuralNetworkRunner.h"
#include "Engine/AssetManager.h"

#include "NNE.h"
#include "NNEModelData.h"

#include "ReinforcementLearning/AICommandsUtils.h"


bool UNeuralNetworkRunner::LoadModel(FONeuralnModelLoaaded OnNeuralModelLoaaded)
{
    if (ModelData.IsNull() || IsModelValid()) return false;

    UAssetManager::GetStreamableManager().RequestAsyncLoad(ModelData.ToSoftObjectPath(), [this, OnNeuralModelLoaaded = MoveTemp(OnNeuralModelLoaaded)]
	{
		bool bSuccess = SetupModel();
		if (ModelData.IsValid()) ModelData.Reset();
		OnNeuralModelLoaaded.ExecuteIfBound(this, bSuccess);
	});

    return true;
}

bool UNeuralNetworkRunner::SendInputs(TArray<float> Inputs, FOnONeuralModelResult OnONeuralModelResult, UObject* Querier)
{
    if (!IsModelValid()) return false;

    AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this, Querier, Inputs = MoveTemp(Inputs), OnONeuralModelResult = MoveTemp(OnONeuralModelResult)]
	{
        auto OnFailure = [this, &OnONeuralModelResult, Querier] { OnONeuralModelResult.ExecuteIfBound({}, this, Querier, false); };
        if (!IsModelValid()) return OnFailure();

		TArray<UE::NNE::FTensorBindingCPU> InputBindings;
		TArray<UE::NNE::FTensorBindingCPU> OutputBindings;

		TArray<float> InputsCopy = Inputs;
		TArray<float> Outputs;

		bool bParamsSet = UAICommandsUtils::SetNeuralModelParams(ModelInstance.Get(), InputBindings, OutputBindings, InputsCopy, Outputs);
		if (!bParamsSet) return OnFailure();

		int32 Status = ModelInstance->RunSync(InputBindings, OutputBindings);
		AsyncTask(ENamedThreads::GameThread, [this, Querier, Status, OnONeuralModelResult, Outputs = MoveTemp(Outputs)]
		{
            bool bSuccess = Status == 0;
			OnONeuralModelResult.ExecuteIfBound(Outputs, this, Querier, bSuccess);
		});
	});
    
    return true;
}

bool UNeuralNetworkRunner::SetupModel()
{
	if (!ModelData.IsValid()) return false;
	TWeakInterfacePtr<INNERuntimeCPU> Runtime = UE::NNE::GetRuntime<INNERuntimeCPU>(TEXT("NNERuntimeORTCpu"));
	if (!Runtime.IsValid()) return false;
	TUniquePtr<UE::NNE::IModelCPU> Model = Runtime->CreateModel(ModelData.Get());
	if (!Model.IsValid()) return false;
	ModelInstance = Model->CreateModelInstance();
	return ModelInstance.IsValid();
}
