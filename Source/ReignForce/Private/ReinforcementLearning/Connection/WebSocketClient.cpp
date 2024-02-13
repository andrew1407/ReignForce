// Fill out your copyright notice in the Description page of Project Settings.


#include "ReinforcementLearning/Connection/WebSocketClient.h"
#include "JsonObjectConverter.h"
#include "JsonUtilities.h"

#include "WebSocketsModule.h" 
#include "IWebSocket.h"


void UWebSocketClient::BeginDestroy()
{
    if (Socket.IsValid()) Socket->Close();

    Super::BeginDestroy();
}

bool UWebSocketClient::IsConnected() const
{
    return Socket.IsValid() && Socket->IsConnected();
}

bool UWebSocketClient::Connect(FClientConnectionOptions Options)
{
    if (IsConnected()) return false;

    const FString Url = FString::Printf(TEXT("ws://%s:%d/"), *Options.Host, Options.Port);
    Socket = FWebSocketsModule::Get().CreateWebSocket(Url);
    
    const TFunction<void(bool)> HandleConnection = [&, OnConnect = Options.OnConnect] (bool bState)
    {
        GetWorld()->GetTimerManager().ClearTimer(ConnectionTimer);
        OnConnect.ExecuteIfBound(bState);
    };

    Socket->OnConnected().AddLambda([HandleConnection] { HandleConnection(true); });

    Socket->OnConnectionError().AddLambda([HandleConnection] (const FString& Error) { HandleConnection(false); });

    Socket->OnClosed().AddLambda([OnDisconnect = MoveTemp(Options.OnDisconnect)] (int32 StatusCode, const FString& Reason, bool bWasClean)
    {
        OnDisconnect.ExecuteIfBound(true);
    });

    Socket->OnMessage().AddLambda([OnMessage = MoveTemp(Options.OnClientResponse)] (const FString& Message)
    {
        FAIClientResponse ResponseData;
        bool bSuccess = FJsonObjectConverter::JsonObjectStringToUStruct<FAIClientResponse>(Message, &ResponseData, 0, 0);
        OnMessage.ExecuteIfBound(MoveTemp(ResponseData), bSuccess);
    });

    Socket->Connect();

    GetWorld()->GetTimerManager()
        .SetTimer(ConnectionTimer, [OnConnect = Options.OnConnect] { OnConnect.ExecuteIfBound(false); }, Options.ConnectionTimeout, false);

    return true;
}

bool UWebSocketClient::Disconnect()
{
    bool bCanDisconnect = IsConnected();
    if (bCanDisconnect)
    {
        Socket->Close();
        Socket.Reset();
    }
    return bCanDisconnect;
}

bool UWebSocketClient::SendInputData(const FAIClientRequest& RequestData)
{
    if (!IsConnected()) return false;
    FString Content;
    bool bConverted = FJsonObjectConverter::UStructToJsonObjectString(RequestData, Content);
    if (bConverted) Socket->Send(Content);
    return bConverted;
}
