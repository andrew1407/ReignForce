// Fill out your copyright notice in the Description page of Project Settings.


#include "ReinforcementLearning/AICommandsUtils.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Algo/MaxElement.h"

#include "ShooterCharacter/ShooterCharacter.h"
#include "Weapons/WeaponType.h"
#include "Weapons/Components/WeaponSlotsSystem.h"


void UAICommandsUtils::SetMetricsByCharacter(AShooterCharacter* ShooterCharacter, FShooterMetrics& Metrics)
{
    constexpr bool bHealthRawFormet = false;
    constexpr bool bHealthPercentageFormet = true;

    Metrics.ShooterState = static_cast<uint8>(ShooterCharacter->GetCombatShooterState());
    Metrics.IsMoving = !ShooterCharacter->GetVelocity().IsNearlyZero();
    Metrics.Health = ShooterCharacter->GetHealthPoints(bHealthRawFormet);
    Metrics.HealthPercent = ShooterCharacter->GetHealthPoints(bHealthPercentageFormet);
    Metrics.ActiveWeapon = static_cast<uint8>(ShooterCharacter->GetSelectedWeaponType());
    Metrics.IsFirearmsActive = UWeaponSlotsSystem::IsFirearmsType(ShooterCharacter->GetSelectedWeaponType());
}

bool UAICommandsUtils::CollectInputData(APawn* CommandExecutor, FAIClientInput& Container, AActor*& Target)
{
    auto ShooterSelf = Cast<AShooterCharacter>(CommandExecutor);
    if (!IsValid(ShooterSelf)) return false;

    auto ExecutorController = ShooterSelf->GetController<AAIController>();
    if (!IsValid(ExecutorController)) return false;

    UBlackboardComponent* ExecutorBlackboard = ExecutorController->GetBlackboardComponent();
    if (!IsValid(ExecutorController)) return false;

    const FName FocusedTargetKey = TEXT("FocusedTarget");
    const FName TargetDistanceKey = TEXT("DistanceToTarget");
    const FName CombatCommandKey = TEXT("CombatCommand");
    const FName IsTargetVisibleKey = TEXT("IsTargetVisible");

    auto TargetShooter = Cast<AShooterCharacter>(ExecutorBlackboard->GetValueAsObject(FocusedTargetKey));
    if (!IsValid(TargetShooter)) return false;
    
    Container.TargetDistance = FVector::Dist(ShooterSelf->GetActorLocation(), TargetShooter->GetActorLocation());
    Container.DistanceState = ExecutorBlackboard->GetValueAsEnum(TargetDistanceKey);
    Container.Command = ExecutorBlackboard->GetValueAsEnum(CombatCommandKey);
    Container.IsTargetVisible = ExecutorBlackboard->GetValueAsBool(IsTargetVisibleKey);
    Container.IsDone = ShooterSelf->IsDead() || TargetShooter->IsDead();

    SetMetricsByCharacter(ShooterSelf, Container.SelfMetrics);
    SetMetricsByCharacter(TargetShooter, Container.TargetMetrics);
    Target = TargetShooter;

    return true;
}

TArray<float> UAICommandsUtils::ConvertShooterMetricsToModelInput(const FShooterMetrics& Data)
{
    return 
    {
        (float)Data.ShooterState,
        (float)Data.IsMoving,
        (float)Data.HealthPercent,
        (float)Data.IsFirearmsActive,
        (float)Data.ActiveWeapon,
    };
}

TArray<float> UAICommandsUtils::ConvertClientInputToModelInput(const FAIClientInput& Data)
{
    TArray<float> CoreInput =
    {
        (float)Data.IsTargetVisible,
        (float)Data.Command,
        (float)Data.DistanceState,
    };
    CoreInput.Append(ConvertShooterMetricsToModelInput(Data.SelfMetrics));
    CoreInput.Append(ConvertShooterMetricsToModelInput(Data.TargetMetrics));
    return MoveTemp(CoreInput);
}

EShooterAICommand UAICommandsUtils::GetPredictedOutputCommand(const TArray<float>& Outputs)
{
    const int32 Index = Outputs.Find(*Algo::MaxElement(Outputs));
    const int32 MaxValue = static_cast<uint8>(EShooterAICommand::Retreat);
    bool bIndexValid = Index > INDEX_NONE && Index <= MaxValue;
    return bIndexValid ? static_cast<EShooterAICommand>(Index) : EShooterAICommand::None;
}

bool UAICommandsUtils::SetNeuralModelParams(UE::NNE::IModelInstanceCPU* ModelInstance, TArray<UE::NNE::FTensorBindingCPU>& InputBindings,
							  TArray<UE::NNE::FTensorBindingCPU>& OutputBindings, TArray<float>& Inputs, TArray<float>& Outputs)
{
    if (!ModelInstance) return false;

    TConstArrayView<UE::NNE::FTensorDesc> InputTensorDescs = ModelInstance->GetInputTensorDescs();
	UE::NNE::FSymbolicTensorShape SymbolicInputTensorShape = InputTensorDescs[0].GetShape();
	TArray<UE::NNE::FTensorShape> InputTensorShapes = { UE::NNE::FTensorShape::MakeFromSymbolic(SymbolicInputTensorShape) };

	ModelInstance->SetInputTensorShapes(InputTensorShapes);

	TConstArrayView<UE::NNE::FTensorDesc> OutputTensorDescs = ModelInstance->GetOutputTensorDescs();
	UE::NNE::FSymbolicTensorShape SymbolicOutputTensorShape = OutputTensorDescs[0].GetShape();
	TArray<UE::NNE::FTensorShape> OutputTensorShapes = { UE::NNE::FTensorShape::MakeFromSymbolic(SymbolicOutputTensorShape) };

	if (Inputs.Num() != InputTensorShapes[0].Volume()) return false;

	InputBindings.SetNumZeroed(1);
	InputBindings[0].Data = Inputs.GetData();
	InputBindings[0].SizeInBytes = Inputs.Num() * sizeof(float);

	Outputs.SetNumZeroed(OutputTensorShapes[0].Volume());
	OutputBindings.SetNumZeroed(1);
	OutputBindings[0].Data = Outputs.GetData();
	OutputBindings[0].SizeInBytes = Outputs.Num() * sizeof(float);

	return true;
}
