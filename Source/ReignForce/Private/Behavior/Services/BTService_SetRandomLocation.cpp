// Fill out your copyright notice in the Description page of Project Settings.

#include "Behavior/Services/BTService_SetRandomLocation.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTService_SetRandomLocation::UBTService_SetRandomLocation(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bNotifyBecomeRelevant = true;
    NodeName = TEXT("Set Random Location");
    BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_SetRandomLocation, BlackboardKey));

    SearchRadius = 500;
}

FString UBTService_SetRandomLocation::GetStaticDescription() const
{
    return FString::Printf(TEXT("Vector: %s"), *BlackboardKey.SelectedKeyName.ToString());
}

void UBTService_SetRandomLocation::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);

    const FVector Origin = OwnerComp.GetAIOwner()->GetPawn()->GetActorLocation();
    const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!IsValid(NavSystem)) return;
    FNavLocation NavLocation;
    bool bLocationFound = NavSystem->GetRandomReachablePointInRadius(Origin, SearchRadius, NavLocation);
    if (bLocationFound)
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(BlackboardKey.SelectedKeyName, NavLocation.Location);
}
