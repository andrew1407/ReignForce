// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/Decorators/BTDecorator_CheckCombatCommand.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTDecorator_CheckCombatCommand::UBTDecorator_CheckCombatCommand(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    NodeName = TEXT("Check Combat Command");
    BlackboardKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckCombatCommand, BlackboardKey), StaticEnum<EShooterAICommand>());
}

FString UBTDecorator_CheckCombatCommand::GetStaticDescription() const
{
    return FString::Printf(TEXT("Value: %s"), *BlackboardKey.SelectedKeyName.ToString());
}

bool UBTDecorator_CheckCombatCommand::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!IsValid(Blackboard)) return false;
    auto CurrentState = static_cast<EShooterAICommand>(Blackboard->GetValueAsEnum(BlackboardKey.SelectedKeyName));
    return ShooterAICommandCheck.Check(CurrentState);
}

