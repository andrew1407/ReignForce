// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/Decorators/BTDecorator_SetRunAwayFrom.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTDecorator_SetRunAwayFrom::UBTDecorator_SetRunAwayFrom(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bNotifyBecomeRelevant = true;
    bNotifyTick = true;
    NodeName = TEXT("Set Run Away From");

    TargetBlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_SetRunAwayFrom, TargetBlackboardKey), AActor::StaticClass());
    DestinationBlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_SetRunAwayFrom, DestinationBlackboardKey));
    DistanceBlackboardKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_SetRunAwayFrom, DistanceBlackboardKey));

    DefaultMovementDistance = 1000;
    RandomPointRadius = 300;
}

FString UBTDecorator_SetRunAwayFrom::GetStaticDescription() const
{
    return FString::Printf(TEXT("Target: %s"), *TargetBlackboardKey.SelectedKeyName.ToString());
}

void UBTDecorator_SetRunAwayFrom::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!IsValid(Blackboard)) return;
    Target = Cast<ACharacter>(Blackboard->GetValueAsObject(TargetBlackboardKey.SelectedKeyName));
}

void UBTDecorator_SetRunAwayFrom::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!IsValid(Blackboard)) return;

    ACharacter* Character = OwnerComp.GetAIOwner()->GetCharacter();
    if (!IsValid(Character)) return;

    const FVector CharacterLocation = Character->GetActorLocation();
    const FVector TargetLocation = Target->GetActorLocation();

    float Distance = Blackboard->GetValueAsFloat(DistanceBlackboardKey.SelectedKeyName);
    if (Distance == 0) Distance = DefaultMovementDistance;
    const FVector Direction = (CharacterLocation - TargetLocation).GetSafeNormal();
    FVector Destination = CharacterLocation + Direction * Distance;
    Destination.Z = CharacterLocation.Z;

    const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    FNavLocation NavLocation;
    bool bLocationFound = NavSystem->GetRandomReachablePointInRadius(Destination, RandomPointRadius, NavLocation);
    if (bLocationFound) Destination = NavLocation.Location;

    Blackboard->SetValueAsVector(DestinationBlackboardKey.SelectedKeyName, Destination);
}
