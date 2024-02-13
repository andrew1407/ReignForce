// Fill out your copyright notice in the Description page of Project Settings.


#include "ReinforcementLearning/RewardEstimators/RewardEstimatorRetreat.h"
#include "ReinforcementLearning/RewardEstimators/AgentReward.h"
#include "ReinforcementLearning/TargetDistance.h"
#include "ReinforcementLearning/ShooterAICommand.h"

#include "Weapons/Components/WeaponSlotsSystem.h"
#include "Weapons/WeaponType.h"
#include "Weapons/ShooterState.h"


float URewardEstimatorRetreat::CalculateReward_Implementation(const FAIClientRequest& Data)
{
    FAgentReward R(0);

    const FAIClientInput& CI = Data.ClientInput;
    const FAIClientInput& PCI = Data.PreviousClientInput;

    auto DistanceType = static_cast<ETargetDistance>(CI.DistanceState);
    auto Command = static_cast<EShooterAICommand>(CI.Command);

    auto ShooterState = static_cast<EShooterState>(CI.SelfMetrics.ShooterState);
    auto TaargetState = static_cast<EShooterState>(CI.TargetMetrics.ShooterState);

    bool bNear = DistanceType == ETargetDistance::Near;

    auto PrevShooterState = static_cast<EShooterState>(PCI.SelfMetrics.ShooterState);
    if (PrevShooterState == EShooterState::Attacking && CI.TargetMetrics.Health < PCI.TargetMetrics.Health) R.Add();

    if (bNear && CI.SelfMetrics.IsFirearmsActive) R.Substract();

    float HealthDifference = CI.TargetMetrics.Health - CI.SelfMetrics.Health;
    if (HealthDifference > 20) R.Substract();
    else if (HealthDifference < 0) R.Add();

    if (Command == EShooterAICommand::IdleAndAttack)
    {
        if (CI.SelfMetrics.IsFirearmsActive && DistanceType == ETargetDistance::Far) R.Add(.5f);
        if (CI.SelfMetrics.IsFirearmsActive) R.Add(.5f);
        R.Add(.3f);
    }
    else if (Command == EShooterAICommand::GetCloseAndAttack)
    {

        if (CI.TargetMetrics.IsFirearmsActive) R.Substract(.5f);
        if (CI.SelfMetrics.IsFirearmsActive && DistanceType == ETargetDistance::Far) R.Add(.5f);
        R.Add(.5f);
    }
    else if (Command == EShooterAICommand::StrafeBackAndAttack)
    {
        if (bNear) R.Add(.5f);

        if (!CI.SelfMetrics.IsFirearmsActive && !bNear) R.Substract();
        if (CI.TargetMetrics.Health < 30) R.Add();
        if (CI.TargetMetrics.IsFirearmsActive) R.Add();
        if (CI.SelfMetrics.IsFirearmsActive && DistanceType == ETargetDistance::Far) R.Add();
        R.Add();
    }
    else if (Command == EShooterAICommand::GetClose)
    {
        if (CI.TargetMetrics.IsFirearmsActive) R.Substract();
        if (CI.SelfMetrics.IsFirearmsActive) R.Add(.5f);
        else R.Substract();
    }
    else if (Command == EShooterAICommand::Idle)
    {
       R.Substract();
    }
    else if (Command == EShooterAICommand::Retreat)
    {
        if (CI.TargetMetrics.Health < 30) R.Add(.5f);
        if (CI.TargetMetrics.IsFirearmsActive) R.Add(.5f);
        R.Add(.5f);
    }
    else if (Command == EShooterAICommand::StrafeBack)
    {
        if (CI.TargetMetrics.IsFirearmsActive) R.Add(.5f);
        if (CI.TargetMetrics.Health < 30) R.Add(.5f);
        R.Add(.5f);
    }

    return R();
}
