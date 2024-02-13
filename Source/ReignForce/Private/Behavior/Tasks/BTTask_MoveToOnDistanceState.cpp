// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/Tasks/BTTask_MoveToOnDistanceState.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTTask_MoveToOnDistanceState::UBTTask_MoveToOnDistanceState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bNotifyTick = true;
    NodeName = TEXT("Move To On Distance State");
    DistanceStateBlackboardKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MoveToOnDistanceState, DistanceStateBlackboardKey), StaticEnum<ETargetDistance>());
    ActivityState = ETargetDistance::None;
}

void UBTTask_MoveToOnDistanceState::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!IsValid(Blackboard)) return;
    auto CurrentState = static_cast<ETargetDistance>(Blackboard->GetValueAsEnum(DistanceStateBlackboardKey.SelectedKeyName));
    if (CurrentState != ActivityState) FinishLatentTask(OwnerComp, EBTNodeResult::Type::Succeeded);
}
