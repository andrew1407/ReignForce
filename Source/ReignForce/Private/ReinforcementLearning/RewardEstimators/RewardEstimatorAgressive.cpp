// Fill out your copyright notice in the Description page of Project Settings.


#include "ReinforcementLearning/RewardEstimators/RewardEstimatorAgressive.h"
#include "ReinforcementLearning/RewardEstimators/AgentReward.h"
#include "ReinforcementLearning/TargetDistance.h"
#include "ReinforcementLearning/ShooterAICommand.h"

#include "Weapons/Components/WeaponSlotsSystem.h"
#include "Weapons/WeaponType.h"
#include "Weapons/ShooterState.h"


float URewardEstimatorAgressive::CalculateReward_Implementation(const FAIClientRequest& Data)
{
    FAgentReward R(0);

    const FAIClientInput& CI = Data.ClientInput;
    const FAIClientInput& PCI = Data.PreviousClientInput;

    auto DistanceType = static_cast<ETargetDistance>(CI.DistanceState);
    auto Command = static_cast<EShooterAICommand>(CI.Command);

    auto ShooterState = static_cast<EShooterState>(CI.SelfMetrics.ShooterState);
    auto TaargetState = static_cast<EShooterState>(CI.TargetMetrics.ShooterState);

    bool bNear = DistanceType == ETargetDistance::Near;

    if (CI.TargetMetrics.Health < PCI.TargetMetrics.Health) R.Add();

    if (Command == EShooterAICommand::IdleAndAttack)
    {
        if (ShooterState == EShooterState::Attacking)
        {
            if (CI.SelfMetrics.IsFirearmsActive) R.Add();
            else if (bNear) R.Add();
            else R.Substract();
        }
    }
    else if (Command == EShooterAICommand::GetCloseAndAttack)
    {
        if (ShooterState == EShooterState::Attacking)
        {
            if (CI.SelfMetrics.IsFirearmsActive) R.Add();
            else if (bNear) R.Substract();
            else R.Add();
        }
    }
    else if (Command == EShooterAICommand::StrafeBackAndAttack)
    {
        if (ShooterState == EShooterState::Attacking)
        {
            if (CI.SelfMetrics.IsFirearmsActive) R.Add();
            else if (!CI.SelfMetrics.IsFirearmsActive && bNear) R.Add();
            else if (!CI.SelfMetrics.IsFirearmsActive && !bNear) R.Substract();
            else R.Add();
        }
    }
    else if (Command == EShooterAICommand::GetClose)
    {
        if (CI.TargetMetrics.IsFirearmsActive && TaargetState == EShooterState::Attacking)
            R.Substract();

        if (bNear) R.Substract();
        else if (!CI.SelfMetrics.IsFirearmsActive) R.Add();
    }
    else if (Command == EShooterAICommand::Idle)
    {
        if (CI.TargetMetrics.IsFirearmsActive && TaargetState == EShooterState::Attacking)
            R.Substract();

        if (bNear) R.Substract();
    }
    else if (Command == EShooterAICommand::Retreat)
    {
        R.Add(0);
    }
    else if (Command == EShooterAICommand::StrafeBack)
    {
        if (DistanceType == ETargetDistance::Far) R.Substract();
        else if (!CI.TargetMetrics.IsFirearmsActive) R.Add(.5f);
        else R.Substract();
    }

    // if (CI.SelfMetrics.Health < 25 && CI.SelfMetrics.Health < CI.TargetMetrics.Health) R.Substract();
    // else if (CI.SelfMetrics.Health > CI.TargetMetrics.Health) R.Add();

    return R();
}
