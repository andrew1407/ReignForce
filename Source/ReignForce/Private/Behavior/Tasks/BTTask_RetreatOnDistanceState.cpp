// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/Tasks/BTTask_RetreatOnDistanceState.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTTask_RetreatOnDistanceState::UBTTask_RetreatOnDistanceState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bNotifyTick = true;
    NodeName = TEXT("Retreat On Distance State");
    DistanceStateBlackboardKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_RetreatOnDistanceState, DistanceStateBlackboardKey), StaticEnum<ETargetDistance>());
    ActivityState = ETargetDistance::None;
}

EBTNodeResult::Type UBTTask_RetreatOnDistanceState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

    bool bSkipCheck = Result == EBTNodeResult::Type::Succeeded || Result == EBTNodeResult::Type::Failed;
    return bSkipCheck ? FinishWithResult(OwnerComp, Result) : CheckState(OwnerComp);
}

void UBTTask_RetreatOnDistanceState::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    CheckState(OwnerComp);
}

EBTNodeResult::Type UBTTask_RetreatOnDistanceState::CheckState(UBehaviorTreeComponent& OwnerComp)
{
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!IsValid(Blackboard)) return FinishWithResult(OwnerComp, EBTNodeResult::Type::Failed);
    auto CurrentState = static_cast<ETargetDistance>(Blackboard->GetValueAsEnum(DistanceStateBlackboardKey.SelectedKeyName));
    EBTNodeResult::Type Result = CurrentState == ActivityState ? EBTNodeResult::Type::InProgress : EBTNodeResult::Type::Succeeded;
    return FinishWithResult(OwnerComp, Result);
}
