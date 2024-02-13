// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/Decorators/BTDecorator_CompareFocusState.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTDecorator_CompareFocusState::UBTDecorator_CompareFocusState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    NodeName = TEXT("Compare Focus State");
    BlackboardKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CompareFocusState, BlackboardKey), StaticEnum<ETargetDistance>());
}

FString UBTDecorator_CompareFocusState::GetStaticDescription() const
{
    return FString::Printf(TEXT("Value: %s"), *BlackboardKey.SelectedKeyName.ToString());
}

bool UBTDecorator_CompareFocusState::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!IsValid(Blackboard)) return false;
    auto CurrentState = static_cast<ETargetDistance>(Blackboard->GetValueAsEnum(BlackboardKey.SelectedKeyName));
    return TargetDistanceCheck.Check(CurrentState);
}
