// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NNERuntimeCPU.h"
#include "NeuralNetworkRunner.generated.h"


class UNNEModelData;
class UNeuralNetworkRunner;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FONeuralnModelLoaaded, UNeuralNetworkRunner*, InstanceSelf, bool, bSuccess);
DECLARE_DYNAMIC_DELEGATE_FourParams(FOnONeuralModelResult, const TArray<float>&, Outputs, UNeuralNetworkRunner*, InstanceSelf, UObject*, Querier, bool, bSuccess);

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class REIGNFORCE_API UNeuralNetworkRunner : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual bool LoadModel(FONeuralnModelLoaaded OnNeuralModelLoaaded);

	UFUNCTION(BlueprintCallable)
	virtual bool SendInputs(TArray<float> Inputs, FOnONeuralModelResult OnONeuralModelResult, UObject* Querier = nullptr);

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsModelValid() const { return ModelInstance.IsValid(); }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Model")
	TSoftObjectPtr<UNNEModelData> ModelData;

	TSharedPtr<UE::NNE::IModelInstanceCPU> ModelInstance;

	/** The model data is being loaded or its instance created (a repeated load request is ignored meanwhile) */
	bool bModelLoading = false;

	/** Model from the loaded data (game thread: the data asset is read), nullptr on failure */
	virtual TSharedPtr<UE::NNE::IModelCPU> CreateModel();

	/**
	 * Creates the model instance (the ONNX runtime session, the heavy part taking up to a second) on a worker thread,
	 * so enemies requesting their models at the round start don't freeze the game; the callback comes on the game thread
	 */
	void CreateModelInstanceAsync(TSharedPtr<UE::NNE::IModelCPU> Model, FONeuralnModelLoaaded OnNeuralModelLoaaded);

	/** Runs the model on a worker thread, the callback is invoked on the game thread (inference failures included) */
	static void RunModelAsync(TSharedPtr<UE::NNE::IModelInstanceCPU> Instance, TArray<float> Inputs, TFunction<void(TArray<float>, bool)> OnGameThreadResult);
};
