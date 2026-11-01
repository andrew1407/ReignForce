#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "ReinforcementLearning/AICommandsUtils.h"
#include "ReinforcementLearning/ShooterAICommandCheck.h"
#include "ReinforcementLearning/TargetDistanceCheck.h"
#include "Weapons/ShooterState.h"
#include "Weapons/WeaponType.h"


namespace
{
    constexpr int32 SHOOTER_METRICS_INPUTS = 5;
    constexpr int32 CLIENT_CORE_INPUTS = 3;

    FShooterMetrics MakeMetrics(EShooterState State, bool bMoving, float HealthPercent, bool bFirearms, EWeaponType Weapon)
    {
        FShooterMetrics Metrics;
        Metrics.ShooterState = static_cast<int32>(State);
        Metrics.IsMoving = bMoving;
        Metrics.Health = HealthPercent * 100;
        Metrics.HealthPercent = HealthPercent;
        Metrics.IsFirearmsActive = bFirearms;
        Metrics.ActiveWeapon = static_cast<int32>(Weapon);
        return Metrics;
    }
}

BEGIN_DEFINE_SPEC(FAICommandsUtilsTests, "ReignForce.Unit.AICommandsUtils",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);
END_DEFINE_SPEC(FAICommandsUtilsTests);

void FAICommandsUtilsTests::Define()
{
    Describe("ModelInput", [this]
    {
        It("ConvertsShooterMetricsInModelOrder", [this]
        {
            const FShooterMetrics Metrics = MakeMetrics(EShooterState::Reloading, true, .25f, true, EWeaponType::Pistol);
            const TArray<float> Input = UAICommandsUtils::ConvertShooterMetricsToModelInput(Metrics);

            if (!TestEqual("Shooter metrics inputs count", Input.Num(), SHOOTER_METRICS_INPUTS)) return;
            TestEqual("[0] shooter state", Input[0], static_cast<float>(EShooterState::Reloading));
            TestEqual("[1] is moving", Input[1], 1.f);
            TestEqual("[2] health percent (not raw health)", Input[2], .25f);
            TestEqual("[3] firearms active", Input[3], 1.f);
            TestEqual("[4] active weapon", Input[4], static_cast<float>(EWeaponType::Pistol));
        });

        It("ConvertsClientInputAsCoreThenSelfThenTarget", [this]
        {
            FAIClientInput ClientInput;
            ClientInput.IsTargetVisible = true;
            ClientInput.Command = static_cast<int32>(EShooterAICommand::StrafeBack);
            ClientInput.DistanceState = static_cast<int32>(ETargetDistance::Near);
            ClientInput.TargetDistance = 1234;  // raw distance & reward aren't model inputs
            ClientInput.Reward = 42;
            ClientInput.SelfMetrics = MakeMetrics(EShooterState::Attacking, false, 1, false, EWeaponType::Melee);
            ClientInput.TargetMetrics = MakeMetrics(EShooterState::Idle, true, .5f, true, EWeaponType::Rifle);

            const TArray<float> Input = UAICommandsUtils::ConvertClientInputToModelInput(ClientInput);
            if (!TestEqual("Model inputs count", Input.Num(), CLIENT_CORE_INPUTS + 2 * SHOOTER_METRICS_INPUTS)) return;

            TestEqual("[0] target visible", Input[0], 1.f);
            TestEqual("[1] command", Input[1], static_cast<float>(EShooterAICommand::StrafeBack));
            TestEqual("[2] distance state", Input[2], static_cast<float>(ETargetDistance::Near));

            const TArray<float> SelfInput = UAICommandsUtils::ConvertShooterMetricsToModelInput(ClientInput.SelfMetrics);
            const TArray<float> TargetInput = UAICommandsUtils::ConvertShooterMetricsToModelInput(ClientInput.TargetMetrics);
            for (int32 I = 0; I < SHOOTER_METRICS_INPUTS; ++I)
            {
                TestEqual(FString::Printf(TEXT("Self metrics input %d"), I), Input[CLIENT_CORE_INPUTS + I], SelfInput[I]);
                TestEqual(FString::Printf(TEXT("Target metrics input %d"), I), Input[CLIENT_CORE_INPUTS + SHOOTER_METRICS_INPUTS + I], TargetInput[I]);
            }
        });
    });

    Describe("PredictedCommand", [this]
    {
        It("PicksTheHighestOutput", [this]
        {
            TestTrue("Max at index 1", UAICommandsUtils::GetPredictedOutputCommand({ .1f, .9f, .3f }) == EShooterAICommand::Idle);
            TestTrue("Max at the last command index", UAICommandsUtils::GetPredictedOutputCommand({ 0, 0, 0, 0, 0, 0, 0, 5 }) == EShooterAICommand::Retreat);
            TestTrue("Negative outputs", UAICommandsUtils::GetPredictedOutputCommand({ -3, -1, -2 }) == EShooterAICommand::Idle);
        });

        It("FallsBackToNoneOutOfCommandsRange", [this]
        {
            TestTrue("Index past the commands", UAICommandsUtils::GetPredictedOutputCommand({ 0, 0, 0, 0, 0, 0, 0, 0, 1 }) == EShooterAICommand::None);
            TestTrue("Index of the None command", UAICommandsUtils::GetPredictedOutputCommand({ 1, 0 }) == EShooterAICommand::None);
        });

        It("HandlesEmptyOutputs", [this]
        {
            // A failed inference delivers no outputs: used to dereference a null max element
            TestTrue("Empty outputs", UAICommandsUtils::GetPredictedOutputCommand({}) == EShooterAICommand::None);
        });
    });

    Describe("InputCollection", [this]
    {
        It("RejectsInvalidExecutor", [this]
        {
            FAIClientInput Container;
            AActor* Target = nullptr;
            TestFalse("No executor", UAICommandsUtils::CollectInputData(nullptr, Container, Target));
            TestNull("No target found", Target);
        });

        It("RejectsModelParamsWithoutModel", [this]
        {
            TArray<UE::NNE::FTensorBindingCPU> InputBindings, OutputBindings;
            TArray<float> Inputs = { 1, 2, 3 }, Outputs;
            TestFalse("No model instance", UAICommandsUtils::SetNeuralModelParams(nullptr, InputBindings, OutputBindings, Inputs, Outputs));
        });
    });

    Describe("CommandChecks", [this]
    {
        It("ComparesSoleCommand", [this]
        {
            FShooterAICommandCheck Check;
            Check.Command = EShooterAICommand::Retreat;
            TestTrue("Same command", Check.Check(EShooterAICommand::Retreat));
            TestFalse("Other command", Check.Check(EShooterAICommand::Idle));
        });

        It("ComparesCommandsSet", [this]
        {
            FShooterAICommandCheck Check;
            Check.bSoleComparison = false;
            Check.Command = EShooterAICommand::Idle;  // ignored in the set mode
            Check.Commands = { EShooterAICommand::GetCloseAndAttack, EShooterAICommand::IdleAndAttack };
            TestTrue("Command in the set", Check.Check(EShooterAICommand::IdleAndAttack));
            TestFalse("Sole command ignored", Check.Check(EShooterAICommand::Idle));
        });

        It("ComparesDistances", [this]
        {
            FTargetDistanceCheck Sole;
            Sole.DistanceState = ETargetDistance::Near;
            TestTrue("Same distance", Sole.Check(ETargetDistance::Near));
            TestFalse("Other distance", Sole.Check(ETargetDistance::Far));

            FTargetDistanceCheck Set;
            Set.bSoleComparison = false;
            Set.DistanceStates = { ETargetDistance::Far, ETargetDistance::InAcceptedBounds };
            TestTrue("Distance in the set", Set.Check(ETargetDistance::Far));
            TestFalse("Distance out of the set", Set.Check(ETargetDistance::Near));
        });
    });
}

#endif
