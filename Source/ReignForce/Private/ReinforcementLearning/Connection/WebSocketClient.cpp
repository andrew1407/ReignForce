// Fill out your copyright notice in the Description page of Project Settings.


#include "ReinforcementLearning/Connection/WebSocketClient.h"
#include "JsonObjectConverter.h"
#include "JsonUtilities.h"

#include "WebSocketsModule.h" 
#include "IWebSocket.h"


void UWebSocketClient::BeginDestroy()
{
    ReleaseSocket();

    Super::BeginDestroy();
}

void UWebSocketClient::ReleaseSocket()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(ConnectionTimer);
    if (!Socket.IsValid()) return;

    // Handlers are dropped before closing: the socket outlives this client in the websockets module until it's closed,
    // its callbacks (and the delegates they captured) mustn't fire into a released client
    Socket->OnConnected().Clear();
    Socket->OnConnectionError().Clear();
    Socket->OnClosed().Clear();
    Socket->OnMessage().Clear();
    if (Socket->IsConnected()) Socket->Close();
    Socket.Reset();
}

bool UWebSocketClient::IsConnected() const
{
    return Socket.IsValid() && Socket->IsConnected();
}

bool UWebSocketClient::Connect(FClientConnectionOptions Options)
{
    if (IsConnected()) return false;

    // A previous (pending / failed) connection socket is released, not leaked with its handlers
    ReleaseSocket();

    const FString Url = FString::Printf(TEXT("ws://%s:%d/"), *Options.Host, Options.Port);
    Socket = FWebSocketsModule::Get().CreateWebSocket(Url);

    const TWeakObjectPtr<UWebSocketClient> WeakSelf(this);
    const TFunction<void(bool)> HandleConnection = [WeakSelf, OnConnect = Options.OnConnect] (bool bState)
    {
        UWebSocketClient* Self = WeakSelf.Get();
        if (!Self) return;
        if (UWorld* World = Self->GetWorld()) World->GetTimerManager().ClearTimer(Self->ConnectionTimer);
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

    if (UWorld* World = GetWorld())
    {
        FTimerDelegate OnTimeout = FTimerDelegate::CreateWeakLambda(this, [OnConnect = Options.OnConnect] { OnConnect.ExecuteIfBound(false); });
        World->GetTimerManager().SetTimer(ConnectionTimer, OnTimeout, Options.ConnectionTimeout, false);
    }

    return true;
}

bool UWebSocketClient::Disconnect()
{
    bool bCanDisconnect = IsConnected();
    if (bCanDisconnect)
    {
        // Closed with the handlers kept, so the disconnection is still reported, then released
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
