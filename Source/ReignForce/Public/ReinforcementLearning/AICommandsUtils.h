// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NNERuntimeCPU.h"

#include "ReinforcementLearning/Connection/AIClientRequest.h"
#include "ReinforcementLearning/ShooterAICommand.h"
#include "ReinforcementLearning/Connection/AIClientRequest.h"
#include "AICommandsUtils.generated.h"


class AShooterCharacter;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UAICommandsUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "AICommandsUtils")
	static void SetMetricsByCharacter(AShooterCharacter* ShooterCharacter, UPARAM(ref) FShooterMetrics& Metrics);

	UFUNCTION(BlueprintPure, Category = "AICommandsUtils")
	static bool CollectInputData(APawn* CommandExecutor, FAIClientInput& Container, AActor*& Target);

	UFUNCTION(BlueprintPure, Category = "AICommandsUtils")
	static TArray<float> ConvertShooterMetricsToModelInput(const FShooterMetrics& Data);

	UFUNCTION(BlueprintPure, Category = "AICommandsUtils")
	static TArray<float> ConvertClientInputToModelInput(const FAIClientInput& Data);

	UFUNCTION(BlueprintPure, Category = "AICommandsUtils")
	static EShooterAICommand GetPredictedOutputCommand(const TArray<float>& Outputs);

	static bool SetNeuralModelParams(UE::NNE::IModelInstanceCPU* ModelInstance, TArray<UE::NNE::FTensorBindingCPU>& InputBindings,
							  		 TArray<UE::NNE::FTensorBindingCPU>& OutputBindings, TArray<float>& Inputs, TArray<float>& Outputs);
};
