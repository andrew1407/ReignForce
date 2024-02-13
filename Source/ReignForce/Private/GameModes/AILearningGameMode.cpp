// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/AILearningGameMode.h"
#include "EngineUtils.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Controller.h"

#include "ReinforcementLearning/AICommandsUtils.h"
#include "ReinforcementLearning/ShooterAICommand.h"
#include "ReinforcementLearning/Interfaces/BehaviourIdentity.h"

#include "ReinforcementLearning/Connection/WebSocketClient.h"
#include "ReinforcementLearning/Connection/AIClientRequest.h"

#include "ReinforcementLearning/RewardEstimators/RewardEstimator.h"
#include "ReinforcementLearning/RewardEstimators/RewardEstimatorAgressive.h"
#include "ReinforcementLearning/RewardEstimators/RewardEstimatorRetreat.h"
#include "ReinforcementLearning/RewardEstimators/RewardEstimatorModerate.h"

#include "GameStates/ShooterGameState.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "Enemy/ShooterAIController.h"

#include "Utils/Cooldown.h"


namespace
{
    bool LOG_AI_LEARING_GAME_MODE_ACTIONS = false;
}

AAILearningGameMode::AAILearningGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	WsClient = CreateDefaultSubobject<UWebSocketClient>(GET_MEMBER_NAME_CHECKED(AAILearningGameMode, WsClient));
    Cooldown = CreateDefaultSubobject<UCooldown>(GET_MEMBER_NAME_CHECKED(AAILearningGameMode, Cooldown));

	Host = TEXT("127.0.0.1");
	Port = 8080;
	ConnectionTimeout = 3;
    RequestFrequency = 1;

    bConnectOnBeginPlay = false;
    StepsPerEpisode = 60;

    InputDataEvent = TEXT("inputs-data");

    InputRewardEstimatorsToCreate =
    {
        { ETemperamentType::Agressive, URewardEstimatorAgressive::StaticClass() },
        { ETemperamentType::Retreat, URewardEstimatorRetreat::StaticClass() },
        { ETemperamentType::Moderate, URewardEstimatorModerate::StaticClass() },
    };
}

void AAILearningGameMode::BeginPlay()
{
    Super::BeginPlay();

    for (const auto &[Type, ToCreate] : InputRewardEstimatorsToCreate)
    {
        if (!ToCreate) continue;
        auto Estimator = NewObject<URewardEstimator>(this, ToCreate);
        if (IsValid(Estimator)) InputRewardEstimators.Add(Type, Estimator);
    }

    if (bConnectOnBeginPlay) CreateClientConnection();
}

bool AAILearningGameMode::RequestCommand_Implementation(APawn* Executor)
{
    bool bCanRequest = IsValid(Executor) && IsValid(WsClient) && WsClient->IsConnected();
    if (!bCanRequest) return false;

    if (Cooldown->HasCooldown(Executor)) return false;
    constexpr float MinRequestFrequencySeconds = .5f;
    Cooldown->Activate(Executor, MinRequestFrequencySeconds);

    AActor* Target = nullptr;
    FAIClientRequest RequestData;
    bool bCollected = UAICommandsUtils::CollectInputData(Executor, RequestData.ClientInput, Target);
	if (!bCollected) return false;

    RequestData.Event = InputDataEvent;
    RequestData.Id = Executor->GetName();

    auto &[Step, PrevInput] = PawnsInput.FindOrAdd(Executor);
    RequestData.IsInitial = Step == 0;
    if (PrevInput.IsDone && !RequestData.IsInitial) return false;

    if (!RequestData.IsInitial) RequestData.PreviousClientInput = PrevInput;
    RequestData.Step = Step++;
    PrevInput = RequestData.ClientInput;
    if (Step % StepsPerEpisode == 0)
    {
        RequestData.ClientInput.IsDone = true;
        Step = 0;
    }

    if (!RequestData.IsInitial)
    {
        URewardEstimator* RewardEstimator = FindEstimatorByIdentity(Executor);
        if (!IsValid(RewardEstimator)) return false;
        RequestData.ClientInput.Reward = RewardEstimator->CalculateReward(RequestData);
    }

    return WsClient->SendInputData(MoveTemp(RequestData));
}

bool AAILearningGameMode::CreateClientConnection()
{
    if (!IsValid(WsClient) || WsClient->IsConnected()) return false;

    FClientConnectionOptions Options;
    Options.Host = Host;
    Options.Port = Port;
    Options.ConnectionTimeout = ConnectionTimeout;

    Options.OnClientResponse.BindDynamic(this, &AAILearningGameMode::OnMessage);
    Options.OnConnect.BindDynamic(this, &AAILearningGameMode::OnConnect);
    Options.OnDisconnect.BindDynamic(this, &AAILearningGameMode::OnDisconnect);

    return WsClient->Connect(MoveTemp(Options));
}

bool AAILearningGameMode::DisconnectClient()
{
    return IsValid(WsClient) && WsClient->Disconnect();
}

void AAILearningGameMode::OnMessage_Implementation(FAIClientResponse ClientResponse, bool bSuccess)
{
    if (!(bSuccess && ClientResponse.IsSuccess)) return;

    if (InputDataEvent == ClientResponse.Event)
    {
        auto ShooterGameState = GetGameState<AShooterGameState>();
        if (!IsValid(ShooterGameState)) return;
        AShooterCharacter* Shooter = ShooterGameState->FindSpawnedEnemyByName(ClientResponse.Id);
        if (!IsValid(Shooter)) return;
        auto AIController = Shooter->GetController<AShooterAIController>();
        if (!IsValid(AIController)) return;
        auto Command = static_cast<EShooterAICommand>(ClientResponse.Command);
        if (Command == EShooterAICommand::None) return;
        AIController->SetShooterAICommand(Command);
    
        if (LOG_AI_LEARING_GAME_MODE_ACTIONS)
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange,
                FString::Printf(TEXT("Event: %s, Step: %d; id: %s; bSuccess: %d; Command: %d"),
                *ClientResponse.Event, ClientResponse.Step, *ClientResponse.Id, ClientResponse.IsSuccess, ClientResponse.Command)
            );
        }
    }
}

void AAILearningGameMode::OnConnect_Implementation(bool bSuccess)
{
    if (LOG_AI_LEARING_GAME_MODE_ACTIONS)
    {
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, bSuccess ? FColor::Green : FColor::Red, FString::Printf(TEXT("WS OnConnect: %d"), bSuccess));
    }
}

void AAILearningGameMode::OnDisconnect_Implementation(bool bSuccess)
{
    if (LOG_AI_LEARING_GAME_MODE_ACTIONS)
    {
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, bSuccess ? FColor::Green : FColor::Red, FString::Printf(TEXT("WS OnDisconnect: %d"), bSuccess));
    }
}

URewardEstimator* AAILearningGameMode::FindEstimatorByIdentity(APawn* Executor) const
{
    if (!IsValid(Executor)) return nullptr;
    AController* ExecutorController = Executor->GetController();
    if (!IsValid(ExecutorController)) return nullptr;

    bool bIdentityInstance = UKismetSystemLibrary::DoesImplementInterface(ExecutorController, UBehaviourIdentity::StaticClass());
    if (!bIdentityInstance) return nullptr;

    ETemperamentType TemperamentType = IBehaviourIdentity::Execute_GetTemperamentType(ExecutorController);
    URewardEstimator* const* RewardEstimator = InputRewardEstimators.Find(TemperamentType);
    return RewardEstimator ? *RewardEstimator : nullptr;
}
