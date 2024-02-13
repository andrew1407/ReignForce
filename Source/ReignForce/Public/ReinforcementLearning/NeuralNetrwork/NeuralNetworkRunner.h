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

	TUniquePtr<UE::NNE::IModelInstanceCPU> ModelInstance;

	virtual bool SetupModel();
};
