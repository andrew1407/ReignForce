// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/ReignForceGameMode.h"
#include "ReinforcementLearning/Interfaces/AICommandsStrategy.h"
#include "ReinforcementLearning/Connection/AIClientResponse.h"
#include "ReinforcementLearning/Connection/AIClientRequest.h"
#include "ReinforcementLearning/TemperamentType.h"
#include "AILearningGameMode.generated.h"


class UWebSocketClient;
class AShooterCharacter;
class URewardEstimator;
class UCooldown;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API AAILearningGameMode : public AReignForceGameMode, public IAICommandsStrategy
{
	GENERATED_BODY()

public:
	AAILearningGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool RequestCommand_Implementation(APawn* Executor) override;

	FORCEINLINE virtual float GetRequestFrequency_Implementation() override { return RequestFrequency; }

	UFUNCTION(BlueprintCallable)
	bool CreateClientConnection();

	UFUNCTION(BlueprintCallable)
	bool DisconnectClient();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent, Category = "AILearning|ConnectionDelegates")
    void OnMessage(FAIClientResponse ClientResponse, bool bSuccess);

    virtual void OnMessage_Implementation(FAIClientResponse ClientResponse, bool bSuccess);

	UFUNCTION(BlueprintNativeEvent, Category = "AILearning|ConnectionDelegates")
    void OnConnect(bool bSuccess);

    virtual void OnConnect_Implementation(bool bSuccess);

	UFUNCTION(BlueprintNativeEvent, Category = "AILearning|ConnectionDelegates")
    void OnDisconnect(bool bSuccess);

    virtual void OnDisconnect_Implementation(bool bSuccess);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AILearning", meta = (AllowPrivateAccess = true))
	bool bConnectOnBeginPlay;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UCooldown> Cooldown;

	#pragma region CONNECTION

	UPROPERTY(BlueprintReadOnly, Category = "AILearning|Connection", meta = (AllowPrivateAccess = true))
	TObjectPtr<UWebSocketClient> WsClient;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AILearning|Connection", meta = (AllowPrivateAccess = true))
	FString Host;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AILearning|Connection", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	int32 Port;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AILearning|Connection", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Seconds"))
	float ConnectionTimeout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AILearning|Connection", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Seconds"))
	float RequestFrequency;

	TMap<APawn*, TPair<int32, FAIClientInput>> PawnsInput;

	#pragma endregion

	#pragma region AI_COMMAND

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AILearning|Command", meta = (AllowPrivateAccess = true))
	FString InputDataEvent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AILearning|Command", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	int32 StepsPerEpisode;

	#pragma endregion

	#pragma region ESTIMATION

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AILearning|Estimation", meta = (AllowPrivateAccess = true))
	TMap<ETemperamentType, TSubclassOf<URewardEstimator>> InputRewardEstimatorsToCreate;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TMap<ETemperamentType, URewardEstimator*> InputRewardEstimators;

	URewardEstimator* FindEstimatorByIdentity(APawn* Executor) const;

	#pragma endregion
};
