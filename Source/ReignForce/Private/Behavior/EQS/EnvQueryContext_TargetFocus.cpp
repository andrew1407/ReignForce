// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/EQS/EnvQueryContext_TargetFocus.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "GameFramework/Actor.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"


UEnvQueryContext_TargetFocus::UEnvQueryContext_TargetFocus(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    TargetBlackboardKey = TEXT("FocusedTarget");
}

void UEnvQueryContext_TargetFocus::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
    if (TargetBlackboardKey.IsNone()) return;
    auto OwnerActor = Cast<AActor>(QueryInstance.Owner.Get());
    if (!IsValid(OwnerActor)) return;
    auto AIController = Cast<AAIController>(OwnerActor->GetInstigatorController());
    if (!IsValid(AIController)) return;
    UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
    if (!IsValid(Blackboard)) return;
    auto Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetBlackboardKey));
    if (IsValid(Target)) UEnvQueryItemType_Actor::SetContextHelper(ContextData, Target);
}
