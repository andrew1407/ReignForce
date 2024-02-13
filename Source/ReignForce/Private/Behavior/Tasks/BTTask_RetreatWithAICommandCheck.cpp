// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/Tasks/BTTask_RetreatWithAICommandCheck.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTTask_RetreatWithAICommandCheck::UBTTask_RetreatWithAICommandCheck(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bNotifyTick = true;
    NodeName = TEXT("Retreat With AI Command Check");
    ShooterAICommandBlackboardKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_RetreatWithAICommandCheck, ShooterAICommandBlackboardKey), StaticEnum<EShooterAICommand>());
}

EBTNodeResult::Type UBTTask_RetreatWithAICommandCheck::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

    bool bSkipCheck = Result == EBTNodeResult::Type::Succeeded || Result == EBTNodeResult::Type::Failed;
    return bSkipCheck ? FinishWithResult(OwnerComp, Result) : CheckState(OwnerComp);
}

void UBTTask_RetreatWithAICommandCheck::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    CheckState(OwnerComp);
}

EBTNodeResult::Type UBTTask_RetreatWithAICommandCheck::CheckState(UBehaviorTreeComponent& OwnerComp)
{
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!IsValid(Blackboard)) return FinishWithResult(OwnerComp, EBTNodeResult::Type::Failed);
    auto CurrentState = static_cast<EShooterAICommand>(Blackboard->GetValueAsEnum(ShooterAICommandBlackboardKey.SelectedKeyName));
    bool bStateFits = ShooterAICommandCheck.Check(CurrentState);
    EBTNodeResult::Type Result = bStateFits ? EBTNodeResult::Type::InProgress : EBTNodeResult::Type::Succeeded;
    return FinishWithResult(OwnerComp, Result);
}

