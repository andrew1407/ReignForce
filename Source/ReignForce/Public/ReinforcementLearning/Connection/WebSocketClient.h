// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ReinforcementLearning/Connection/AIClientRequest.h"
#include "ReinforcementLearning/Connection/AIClientResponse.h"
#include "WebSocketClient.generated.h"


class IWebSocket;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnClientConnection, bool, IsSuccess);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnClientResponse, FAIClientResponse, ClientResponse, bool, IsSuccess);

USTRUCT(BlueprintType)
struct FClientConnectionOptions
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Host;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	int32 Port;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, Units = "Seconds"))
	float ConnectionTimeout;

    UPROPERTY(BlueprintReadWrite)
	FOnClientResponse OnClientResponse;

    UPROPERTY(BlueprintReadWrite)
	FOnClientConnection OnConnect;

	UPROPERTY(BlueprintReadWrite)
	FOnClientConnection OnDisconnect;
};

/**
 * 
 */
UCLASS(EditInlineNew, BlueprintType)
class REIGNFORCE_API UWebSocketClient : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	bool IsConnected() const;

	UFUNCTION(BlueprintCallable)
	bool Connect(FClientConnectionOptions Options);

	UFUNCTION(BlueprintCallable)
	bool Disconnect();

	UFUNCTION(BlueprintCallable)
	bool SendInputData(const FAIClientRequest& RequestData);

protected:
	virtual void BeginDestroy() override;

private:
	UPROPERTY()
	FTimerHandle ConnectionTimer;

	TSharedPtr<IWebSocket> Socket;
};
