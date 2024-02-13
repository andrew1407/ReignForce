// Fill out your copyright notice in the Description page of Project Settings.


#include "ReinforcementLearning/RewardEstimators/RewardEstimatorModerate.h"
#include "ReinforcementLearning/RewardEstimators/AgentReward.h"
#include "ReinforcementLearning/TargetDistance.h"
#include "ReinforcementLearning/ShooterAICommand.h"

#include "Weapons/Components/WeaponSlotsSystem.h"
#include "Weapons/WeaponType.h"
#include "Weapons/ShooterState.h"


float URewardEstimatorModerate::CalculateReward_Implementation(const FAIClientRequest& Data)
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

    if (Command == EShooterAICommand::IdleAndAttack)
    {
        if (ShooterState == EShooterState::Attacking)
        {
            if (CI.SelfMetrics.IsFirearmsActive) R.Add();
            else bNear ? R.Add() : R.Substract();
        }
        else if (bNear)
        {
            R.Substract();
        }
    }
    else if (Command == EShooterAICommand::GetCloseAndAttack)
    {
        if (CI.TargetMetrics.IsFirearmsActive) R.Substract();
        if (ShooterState == EShooterState::Attacking)
        {
            if (CI.SelfMetrics.IsFirearmsActive) R.Add();
            else bNear ? R.Add() : R.Substract();
        }
        else
        {
            if (bNear) R.Substract();
            else R.Add(.5f);
        }
    }
    else if (Command == EShooterAICommand::StrafeBackAndAttack)
    {
        if (!CI.SelfMetrics.IsFirearmsActive && !bNear) R.Substract();
        if (ShooterState == EShooterState::Attacking)
        {
            if (CI.SelfMetrics.IsFirearmsActive) R.Add();
            else bNear ? R.Add() : R.Substract();
        }
        else
        {
            if (bNear) R.Substract();
            else R.Add(.5f);
        }

        if (!CI.SelfMetrics.IsFirearmsActive && CI.TargetMetrics.Health < 30) R.Add();
        else if (CI.SelfMetrics.IsFirearmsActive && CI.TargetMetrics.Health < 30) R.Add(.5f);

        if (!CI.SelfMetrics.IsFirearmsActive && CI.TargetMetrics.IsFirearmsActive) R.Add();
        else if (CI.SelfMetrics.IsFirearmsActive && CI.TargetMetrics.IsFirearmsActive) R.Add(.5f);
    }
    else if (Command == EShooterAICommand::GetClose)
    {
        if (CI.SelfMetrics.IsFirearmsActive) R.Add(.5f);
    }
    else if (Command == EShooterAICommand::Idle)
    {
        R.Substract();
    }
    else if (Command == EShooterAICommand::Retreat)
    {
        if (CI.TargetMetrics.Health < 30) R.Add(.3f);
        if (CI.TargetMetrics.IsFirearmsActive) R.Add(.3f);
    }
    else if (Command == EShooterAICommand::StrafeBack)
    {
        if (CI.TargetMetrics.Health < 30) R.Add(.5f);
        if (CI.TargetMetrics.IsFirearmsActive) R.Add(.5f);
    }

    // if (CI.SelfMetrics.Health < 25 && CI.SelfMetrics.Health < CI.TargetMetrics.Health) R.Substract();
    // else if (CI.SelfMetrics.Health > CI.TargetMetrics.Health) R.Add();

    return R();
}
