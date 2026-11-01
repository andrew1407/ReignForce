// Fill out your copyright notice in the Description page of Project Settings.


#include "ReinforcementLearning/NeuralNetrwork/NeuralNetworkRunner.h"
#include "Engine/AssetManager.h"

#include "NNE.h"
#include "NNEModelData.h"

#include "ReinforcementLearning/AICommandsUtils.h"


bool UNeuralNetworkRunner::LoadModel(FONeuralnModelLoaaded OnNeuralModelLoaaded)
{
    if (ModelData.IsNull() || IsModelValid() || bModelLoading) return false;
    bModelLoading = true;

    // Weak self: the runner may be collected (level change) before the async load finishes
    UAssetManager::GetStreamableManager().RequestAsyncLoad(ModelData.ToSoftObjectPath(), [WeakSelf = TWeakObjectPtr<UNeuralNetworkRunner>(this), OnNeuralModelLoaaded = MoveTemp(OnNeuralModelLoaaded)]
	{
		UNeuralNetworkRunner* Self = WeakSelf.Get();
		if (!Self) return;
		TSharedPtr<UE::NNE::IModelCPU> Model = Self->CreateModel();
		if (Self->ModelData.IsValid()) Self->ModelData.Reset();
		Self->CreateModelInstanceAsync(MoveTemp(Model), OnNeuralModelLoaaded);
	});

    return true;
}

TSharedPtr<UE::NNE::IModelCPU> UNeuralNetworkRunner::CreateModel()
{
	if (!ModelData.IsValid()) return nullptr;
	TWeakInterfacePtr<INNERuntimeCPU> Runtime = UE::NNE::GetRuntime<INNERuntimeCPU>(TEXT("NNERuntimeORTCpu"));
	if (!Runtime.IsValid()) return nullptr;
	return Runtime->CreateModelCPU(ModelData.Get());
}

void UNeuralNetworkRunner::CreateModelInstanceAsync(TSharedPtr<UE::NNE::IModelCPU> Model, FONeuralnModelLoaaded OnNeuralModelLoaaded)
{
	if (!Model.IsValid())
	{
		bModelLoading = false;
		OnNeuralModelLoaaded.ExecuteIfBound(this, false);
		return;
	}

	// The worker only touches the model (kept alive by the shared pointer), the runner is accessed back on the game thread if still alive
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [Model = MoveTemp(Model), WeakSelf = TWeakObjectPtr<UNeuralNetworkRunner>(this), OnNeuralModelLoaaded = MoveTemp(OnNeuralModelLoaaded)]() mutable
	{
		TSharedPtr<UE::NNE::IModelInstanceCPU> Instance = Model->CreateModelInstanceCPU();
		AsyncTask(ENamedThreads::GameThread, [Instance = MoveTemp(Instance), WeakSelf, OnNeuralModelLoaaded = MoveTemp(OnNeuralModelLoaaded)]() mutable
		{
			UNeuralNetworkRunner* Self = WeakSelf.Get();
			if (!Self) return;
			Self->ModelInstance = MoveTemp(Instance);
			Self->bModelLoading = false;
			OnNeuralModelLoaaded.ExecuteIfBound(Self, Self->IsModelValid());
		});
	});
}

bool UNeuralNetworkRunner::SendInputs(TArray<float> Inputs, FOnONeuralModelResult OnONeuralModelResult, UObject* Querier)
{
    if (!IsModelValid()) return false;

    // The worker owns a model instance reference (kept alive even if the runner is collected meanwhile) and never touches
    // UObjects: results (including failures) are delivered on the game thread only if the runner & querier are still alive
    RunModelAsync(ModelInstance, MoveTemp(Inputs), [WeakSelf = TWeakObjectPtr<UNeuralNetworkRunner>(this), WeakQuerier = TWeakObjectPtr<UObject>(Querier), bHasQuerier = Querier != nullptr,
        OnONeuralModelResult = MoveTemp(OnONeuralModelResult)] (TArray<float> Outputs, bool bSuccess)
    {
        UNeuralNetworkRunner* Self = WeakSelf.Get();
        UObject* QuerierObject = WeakQuerier.Get();
        if (!Self || (bHasQuerier && !QuerierObject)) return;
        OnONeuralModelResult.ExecuteIfBound(Outputs, Self, QuerierObject, bSuccess);
    });

    return true;
}

void UNeuralNetworkRunner::RunModelAsync(TSharedPtr<UE::NNE::IModelInstanceCPU> Instance, TArray<float> Inputs, TFunction<void(TArray<float>, bool)> OnGameThreadResult)
{
    AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [Instance = MoveTemp(Instance), Inputs = MoveTemp(Inputs), OnGameThreadResult = MoveTemp(OnGameThreadResult)]() mutable
	{
		TArray<UE::NNE::FTensorBindingCPU> InputBindings;
		TArray<UE::NNE::FTensorBindingCPU> OutputBindings;
		TArray<float> Outputs;

		bool bSuccess = Instance.IsValid() && UAICommandsUtils::SetNeuralModelParams(Instance.Get(), InputBindings, OutputBindings, Inputs, Outputs);
		if (bSuccess) bSuccess = Instance->RunSync(InputBindings, OutputBindings) == UE::NNE::EResultStatus::Ok;
		if (!bSuccess) Outputs.Reset();

		AsyncTask(ENamedThreads::GameThread, [OnGameThreadResult = MoveTemp(OnGameThreadResult), Outputs = MoveTemp(Outputs), bSuccess]() mutable
		{
			OnGameThreadResult(MoveTemp(Outputs), bSuccess);
		});
	});
}
