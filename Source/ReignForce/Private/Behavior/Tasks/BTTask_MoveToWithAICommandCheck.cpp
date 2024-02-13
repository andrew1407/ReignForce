// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/Tasks/BTTask_MoveToWithAICommandCheck.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTTask_MoveToWithAICommandCheck::UBTTask_MoveToWithAICommandCheck(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bNotifyTick = true;
    NodeName = TEXT("Move To With AI Command Check");
    ShooterAICommandBlackboardKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MoveToWithAICommandCheck, ShooterAICommandBlackboardKey), StaticEnum<EShooterAICommand>());
}

void UBTTask_MoveToWithAICommandCheck::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!IsValid(Blackboard)) return;
    auto CurrentState = static_cast<EShooterAICommand>(Blackboard->GetValueAsEnum(ShooterAICommandBlackboardKey.SelectedKeyName));
    bool bAppropriateStateStillSet = ShooterAICommandCheck.Check(CurrentState);
    if (!bAppropriateStateStillSet) FinishLatentTask(OwnerComp, EBTNodeResult::Type::Succeeded);
}
