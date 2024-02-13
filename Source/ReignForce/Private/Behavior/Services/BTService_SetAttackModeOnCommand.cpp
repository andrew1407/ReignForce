// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/Services/BTService_SetAttackModeOnCommand.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ShooterCharacter/ShooterCharacter.h"


UBTService_SetAttackModeOnCommand::UBTService_SetAttackModeOnCommand(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bNotifyBecomeRelevant = true;
    NodeName = TEXT("Set Attack Mod On Command");
    BlackboardKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_SetAttackModeOnCommand, BlackboardKey), StaticEnum<EShooterAICommand>());

    bMode = true;
    ShooterAICommandCheck.bSoleComparison = false;
    ShooterAICommandCheck.Commands =
    {
        EShooterAICommand::IdleAndAttack,
        EShooterAICommand::GetCloseAndAttack,
        EShooterAICommand::StrafeBackAndAttack,
    };
}

FString UBTService_SetAttackModeOnCommand::GetStaticDescription() const
{
    return FString::Printf(TEXT("AI Comamnd: %s"), *BlackboardKey.SelectedKeyName.ToString());
}

void UBTService_SetAttackModeOnCommand::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!IsValid(Blackboard)) return;

    auto Shooter = Cast<AShooterCharacter>(OwnerComp.GetAIOwner()->GetCharacter());
    if (!IsValid(Shooter)) return;

    auto CurrentState = static_cast<EShooterAICommand>(Blackboard->GetValueAsEnum(BlackboardKey.SelectedKeyName));
    bool bSTateFits = ShooterAICommandCheck.Check(CurrentState);
    if (!bMode) bSTateFits = !bSTateFits;

    if (bSTateFits) Shooter->StartAttacking();
    else Shooter->StopAttacking();
}

