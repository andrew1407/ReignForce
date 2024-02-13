// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/AIModelsGameMode.h"
#include "EngineUtils.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Controller.h"

#include "ReinforcementLearning/NeuralNetrwork/NeuralNetworkRunner.h"
#include "ReinforcementLearning/Connection/AIClientRequest.h"
#include "ReinforcementLearning/Interfaces/BehaviourIdentity.h"
#include "ReinforcementLearning/AICommandsUtils.h"
#include "ReinforcementLearning/ShooterAICommand.h"

#include "ShooterCharacter/ShooterCharacter.h"
#include "Enemy/ShooterAIController.h"

#include "Utils/Cooldown.h"


namespace
{
    bool LOG_AI_MODELS_GAME_MODE_ACTIONS = false;
}

AAIModelsGameMode::AAIModelsGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    Cooldown = CreateDefaultSubobject<UCooldown>(GET_MEMBER_NAME_CHECKED(AAIModelsGameMode, Cooldown));

    RequestFrequency = 1;
    bConnectOnBeginPlay = false;
}

void AAIModelsGameMode::BeginPlay()
{
    Super::BeginPlay();

    for (const auto &[Type, ToCreate] : NeuralNetworkToCreate)
    {
        if (!ToCreate) continue;
        auto Network = NewObject<UNeuralNetworkRunner>(this, ToCreate);
        if (IsValid(Network)) NeuralNetworkRunners.Add(Type, Network);
    }

    if (bConnectOnBeginPlay) LoadAllAvailaleNeuralModels();
}

bool AAIModelsGameMode::RequestCommand_Implementation(APawn* Executor)
{
    if (Cooldown->HasCooldown(Executor)) return false;
    constexpr float MinRequestFrequencySeconds = .5f;
    Cooldown->Activate(Executor, MinRequestFrequencySeconds);

    UNeuralNetworkRunner* NetworkRunner = FindNeuralNetworkByIdentity(Executor);
    if (!IsValid(NetworkRunner)) return false;

    FAIClientInput ClientInput;
    AActor* Target = nullptr;
    bool bCollected = UAICommandsUtils::CollectInputData(Executor, ClientInput, Target);
	if (!bCollected) return false;

    TArray<float> InputsData = UAICommandsUtils::ConvertClientInputToModelInput(ClientInput);
    FOnONeuralModelResult OnONeuralModelResult;
    OnONeuralModelResult.BindDynamic(this, &AAIModelsGameMode::OnNetworkRunnerResult);

    return NetworkRunner->SendInputs(InputsData, MoveTemp(OnONeuralModelResult), Executor);
}

bool AAIModelsGameMode::LoadAllAvailaleNeuralModels()
{
    FONeuralnModelLoaaded OnNeuralModelLoaaded;
    OnNeuralModelLoaaded.BindDynamic(this, &AAIModelsGameMode::OnNetworkRunnerConnect);

    for (const auto &[_, NetworkRunner] : NeuralNetworkRunners)
    {
        if (!IsValid(NetworkRunner)) return false;
        bool bCanConnect = NetworkRunner->LoadModel(OnNeuralModelLoaaded);
        if (!bCanConnect) return false;
    }

    return true;
}

bool AAIModelsGameMode::LoadModel(ETemperamentType TemperamentType)
{
    UNeuralNetworkRunner** RunnerPtr = NeuralNetworkRunners.Find(TemperamentType);
    if (!(RunnerPtr && IsValid(*RunnerPtr))) return false;

    FONeuralnModelLoaaded OnNeuralModelLoaaded;
    OnNeuralModelLoaaded.BindDynamic(this, &AAIModelsGameMode::OnNetworkRunnerConnect);

    return (*RunnerPtr)->LoadModel(MoveTemp(OnNeuralModelLoaaded));
}

void AAIModelsGameMode::OnNetworkRunnerConnect_Implementation(UNeuralNetworkRunner* NetworkRunner, bool bSuccess)
{
    if (LOG_AI_MODELS_GAME_MODE_ACTIONS)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, bSuccess ? FColor::Green : FColor::Red,
            FString::Printf(TEXT("\"%s\" OnConnect: %d"),
            *GetNameSafe(NetworkRunner), bSuccess)
        );
    }
}

void AAIModelsGameMode::OnNetworkRunnerResult_Implementation(const TArray<float>& Outputs, UNeuralNetworkRunner* NetworkRunner, UObject* Querier, bool bSuccess)
{
    auto Shooter = Cast<AShooterCharacter>(Querier);
    if (!IsValid(Shooter)) return;
    auto AIController = Shooter->GetController<AShooterAIController>();
    if (!IsValid(AIController)) return;
    EShooterAICommand Command = UAICommandsUtils::GetPredictedOutputCommand(Outputs);
    if (Command == EShooterAICommand::None) return;
    AIController->SetShooterAICommand(Command);

    if (LOG_AI_MODELS_GAME_MODE_ACTIONS)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, bSuccess ? FColor::Green : FColor::Red,
            FString::Printf(TEXT("NetworkRunner: %s; bSuccess: %d; Querier: %s; Command: %d"),
            *GetNameSafe(NetworkRunner), bSuccess, *GetNameSafe(Querier), Command)
        );
    }
}

UNeuralNetworkRunner* AAIModelsGameMode::FindNeuralNetworkByIdentity(APawn* Executor) const
{
    if (!IsValid(Executor)) return nullptr;
    AController* ExecutorController = Executor->GetController();
    if (!IsValid(ExecutorController)) return nullptr;

    bool bIdentityInstance = UKismetSystemLibrary::DoesImplementInterface(ExecutorController, UBehaviourIdentity::StaticClass());
    if (!bIdentityInstance) return nullptr;

    ETemperamentType TemperamentType = IBehaviourIdentity::Execute_GetTemperamentType(ExecutorController);
    UNeuralNetworkRunner* const* NetworkRunner = NeuralNetworkRunners.Find(TemperamentType);
    return NetworkRunner ? *NetworkRunner : nullptr;
}
