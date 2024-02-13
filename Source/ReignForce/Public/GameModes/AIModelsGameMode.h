// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/ReignForceGameMode.h"
#include "ReinforcementLearning/TemperamentType.h"
#include "ReinforcementLearning/Interfaces/AICommandsStrategy.h"
#include "AIModelsGameMode.generated.h"


class UNeuralNetworkRunner;
class UCooldown;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API AAIModelsGameMode : public AReignForceGameMode, public IAICommandsStrategy
{
	GENERATED_BODY()

public:
	AAIModelsGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool RequestCommand_Implementation(APawn* Executor) override;

	FORCEINLINE virtual float GetRequestFrequency_Implementation() override { return RequestFrequency; }

	UFUNCTION(BlueprintCallable)
	bool LoadAllAvailaleNeuralModels();

	UFUNCTION(BlueprintCallable)
	bool LoadModel(ETemperamentType TemperamentType);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent, Category = "NeuralNetwork|Delegates")
	void OnNetworkRunnerConnect(UNeuralNetworkRunner* NetworkRunner, bool bSuccess);

	virtual void OnNetworkRunnerConnect_Implementation(UNeuralNetworkRunner* NetworkRunner, bool bSuccess);

	UFUNCTION(BlueprintNativeEvent, Category = "NeuralNetwork|Delegates")
	void OnNetworkRunnerResult(const TArray<float>& Outputs, UNeuralNetworkRunner* NetworkRunner, UObject* Querier, bool bSuccess);
	
	virtual void OnNetworkRunnerResult_Implementation(const TArray<float>& Outputs, UNeuralNetworkRunner* NetworkRunner, UObject* Querier, bool bSuccess);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeuralNetwork", meta = (AllowPrivateAccess = true))
	bool bConnectOnBeginPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NeuralNetwork", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Seconds"))
	float RequestFrequency;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UCooldown> Cooldown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeuralNetwork", meta = (AllowPrivateAccess = true))
	TMap<ETemperamentType, TSubclassOf<UNeuralNetworkRunner>> NeuralNetworkToCreate;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TMap<ETemperamentType, UNeuralNetworkRunner*> NeuralNetworkRunners;

	UNeuralNetworkRunner* FindNeuralNetworkByIdentity(APawn* Executor) const;
};
