#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "ReinforcementLearning/RewardEstimators/AgentReward.h"
#include "ReinforcementLearning/RewardEstimators/RewardEstimatorAgressive.h"
#include "ReinforcementLearning/RewardEstimators/RewardEstimatorModerate.h"
#include "ReinforcementLearning/RewardEstimators/RewardEstimatorRetreat.h"
#include "ReinforcementLearning/ShooterAICommand.h"
#include "ReinforcementLearning/TargetDistance.h"
#include "Weapons/ShooterState.h"


namespace
{
    constexpr float REWARD_TOLERANCE = 1e-4f;

    struct FRequestBuilder
    {
        FAIClientRequest Request;

        FRequestBuilder(EShooterAICommand Command, ETargetDistance Distance)
        {
            Request.ClientInput.Command = static_cast<int32>(Command);
            Request.ClientInput.DistanceState = static_cast<int32>(Distance);
            Request.ClientInput.SelfMetrics.Health = Request.PreviousClientInput.SelfMetrics.Health = 100;
            Request.ClientInput.TargetMetrics.Health = Request.PreviousClientInput.TargetMetrics.Health = 100;
        }

        FRequestBuilder& Self(EShooterState State, bool bFirearms, float Health = 100)
        {
            Request.ClientInput.SelfMetrics.ShooterState = static_cast<int32>(State);
            Request.ClientInput.SelfMetrics.IsFirearmsActive = bFirearms;
            Request.ClientInput.SelfMetrics.Health = Health;
            return *this;
        }

        FRequestBuilder& Target(EShooterState State, bool bFirearms, float Health = 100)
        {
            Request.ClientInput.TargetMetrics.ShooterState = static_cast<int32>(State);
            Request.ClientInput.TargetMetrics.IsFirearmsActive = bFirearms;
            Request.ClientInput.TargetMetrics.Health = Health;
            return *this;
        }

        FRequestBuilder& Previous(EShooterState SelfState, float TargetHealth)
        {
            Request.PreviousClientInput.SelfMetrics.ShooterState = static_cast<int32>(SelfState);
            Request.PreviousClientInput.TargetMetrics.Health = TargetHealth;
            return *this;
        }
    };

    template <typename TEstimator>
    float Estimate(const FAIClientRequest& Request)
    {
        auto Estimator = NewObject<TEstimator>(GetTransientPackage());
        return Estimator->CalculateReward(Request);
    }
}

BEGIN_DEFINE_SPEC(FRewardEstimatorsTests, "ReignForce.Unit.RewardEstimators",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);
END_DEFINE_SPEC(FRewardEstimatorsTests);

void FRewardEstimatorsTests::Define()
{
    Describe("AgentReward", [this]
    {
        It("AccumulatesAddedAndSubstractedValues", [this]
        {
            FAgentReward Reward;
            TestEqual("Reward starts at zero", Reward(), 0.f);

            Reward.Add();
            Reward.Add(.5f);
            Reward.Substract(2);
            TestEqual("1 + .5 - 2", Reward(), -.5f, REWARD_TOLERANCE);

            *Reward = 3;
            TestEqual("Dereferenced value is assignable", Reward(), 3.f);
        });
    });

    Describe("Base", [this]
    {
        It("GivesNoReward", [this]
        {
            const FAIClientRequest Request = FRequestBuilder(EShooterAICommand::GetCloseAndAttack, ETargetDistance::Near).Request;
            TestEqual("Base estimator reward", Estimate<URewardEstimator>(Request), 0.f);
        });
    });

    Describe("Agressive", [this]
    {
        It("RewardsFirearmsAttackWithTargetHit", [this]
        {
            const FAIClientRequest Request = FRequestBuilder(EShooterAICommand::IdleAndAttack, ETargetDistance::Far)
                .Self(EShooterState::Attacking, true).Target(EShooterState::Idle, false, 60).Previous(EShooterState::Idle, 80).Request;
            TestEqual("Target damaged (+1), firearms attack (+1)", Estimate<URewardEstimatorAgressive>(Request), 2.f, REWARD_TOLERANCE);
        });

        It("PunishesMeleeApproachWhenAlreadyNear", [this]
        {
            const FAIClientRequest Request = FRequestBuilder(EShooterAICommand::GetCloseAndAttack, ETargetDistance::Near)
                .Self(EShooterState::Attacking, false).Request;
            TestEqual("Getting close with melee being near (-1)", Estimate<URewardEstimatorAgressive>(Request), -1.f, REWARD_TOLERANCE);
        });

        It("IsNeutralToRetreat", [this]
        {
            const FAIClientRequest Request = FRequestBuilder(EShooterAICommand::Retreat, ETargetDistance::Far).Request;
            TestEqual("Retreat", Estimate<URewardEstimatorAgressive>(Request), 0.f, REWARD_TOLERANCE);
        });

        It("JudgesStrafingBackByDistanceAndTargetWeapon", [this]
        {
            const FAIClientRequest FarRequest = FRequestBuilder(EShooterAICommand::StrafeBack, ETargetDistance::Far).Request;
            TestEqual("Strafing back being far (-1)", Estimate<URewardEstimatorAgressive>(FarRequest), -1.f, REWARD_TOLERANCE);

            const FAIClientRequest MeleeTargetRequest = FRequestBuilder(EShooterAICommand::StrafeBack, ETargetDistance::Near)
                .Target(EShooterState::Idle, false).Request;
            TestEqual("Strafing back from a melee target (+.5)", Estimate<URewardEstimatorAgressive>(MeleeTargetRequest), .5f, REWARD_TOLERANCE);
        });

        It("PunishesApproachUnderFire", [this]
        {
            const FAIClientRequest Request = FRequestBuilder(EShooterAICommand::GetClose, ETargetDistance::Near)
                .Target(EShooterState::Attacking, true).Request;
            TestEqual("Shot while getting close (-1) being near (-1)", Estimate<URewardEstimatorAgressive>(Request), -2.f, REWARD_TOLERANCE);
        });
    });

    Describe("Moderate", [this]
    {
        It("PunishesIdling", [this]
        {
            const FAIClientRequest Request = FRequestBuilder(EShooterAICommand::Idle, ETargetDistance::Far).Request;
            TestEqual("Idle (-1)", Estimate<URewardEstimatorModerate>(Request), -1.f, REWARD_TOLERANCE);
        });

        It("RewardsRetreatFromWeakArmedTarget", [this]
        {
            const FAIClientRequest Request = FRequestBuilder(EShooterAICommand::Retreat, ETargetDistance::Far)
                .Target(EShooterState::Idle, true, 20).Previous(EShooterState::Idle, 20).Request;
            TestEqual("Weak target (+.3), armed target (+.3)", Estimate<URewardEstimatorModerate>(Request), .6f, REWARD_TOLERANCE);
        });

        It("RewardsHitOnlyIfAttackedBefore", [this]
        {
            FRequestBuilder Builder(EShooterAICommand::IdleAndAttack, ETargetDistance::Far);
            Builder.Self(EShooterState::Attacking, true).Target(EShooterState::Idle, false, 50);

            const FAIClientRequest AttackedBefore = FRequestBuilder(Builder).Previous(EShooterState::Attacking, 70).Request;
            TestEqual("Hit after attacking (+1), firearms attack (+1)", Estimate<URewardEstimatorModerate>(AttackedBefore), 2.f, REWARD_TOLERANCE);

            const FAIClientRequest IdleBefore = FRequestBuilder(Builder).Previous(EShooterState::Idle, 70).Request;
            TestEqual("Target damage not caused by self, firearms attack (+1)", Estimate<URewardEstimatorModerate>(IdleBefore), 1.f, REWARD_TOLERANCE);
        });

        It("PunishesApproachToArmedTargetWithoutAttack", [this]
        {
            const FAIClientRequest Request = FRequestBuilder(EShooterAICommand::GetCloseAndAttack, ETargetDistance::Near)
                .Self(EShooterState::Idle, false).Target(EShooterState::Idle, true).Request;
            TestEqual("Armed target (-1), no attack being near (-1)", Estimate<URewardEstimatorModerate>(Request), -2.f, REWARD_TOLERANCE);
        });
    });

    Describe("Retreat", [this]
    {
        It("PunishesIdling", [this]
        {
            const FAIClientRequest Request = FRequestBuilder(EShooterAICommand::Idle, ETargetDistance::InAcceptedBounds).Request;
            TestEqual("Idle (-1)", Estimate<URewardEstimatorRetreat>(Request), -1.f, REWARD_TOLERANCE);
        });

        It("RewardsRetreatWhenHealthier", [this]
        {
            const FAIClientRequest Request = FRequestBuilder(EShooterAICommand::Retreat, ETargetDistance::Far)
                .Self(EShooterState::Idle, false, 100).Target(EShooterState::Idle, false, 50).Previous(EShooterState::Idle, 50).Request;
            TestEqual("Healthier than target (+1), retreat (+.5)", Estimate<URewardEstimatorRetreat>(Request), 1.5f, REWARD_TOLERANCE);
        });

        It("PunishesCloseFirearmsFightWhenWeaker", [this]
        {
            const FAIClientRequest Request = FRequestBuilder(EShooterAICommand::GetClose, ETargetDistance::Near)
                .Self(EShooterState::Idle, true, 50).Target(EShooterState::Idle, false, 100).Request;
            TestEqual("Firearms near (-1), much weaker (-1), firearms approach (+.5)", Estimate<URewardEstimatorRetreat>(Request), -1.5f, REWARD_TOLERANCE);
        });
    });
}

#endif
