// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/Decorators/BTDecorator_CheckTargetDistance.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "ShooterCharacter/ShooterCharacter.h"
#include "Weapons/Components/WeaponSlotsSystem.h"
#include "ReinforcementLearning/TargetDistance.h"


UBTDecorator_CheckTargetDistance::UBTDecorator_CheckTargetDistance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bNotifyBecomeRelevant = true;
    bNotifyTick = true;

    NodeName = TEXT("Check Target Distance");
    TargetBlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckTargetDistance, TargetBlackboardKey), AActor::StaticClass());
    DistanceStateBlackboardKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckTargetDistance, DistanceStateBlackboardKey), StaticEnum<ETargetDistance>());

    DefaultDistance = { 300, 500 };
    Distances =
    {
        { EWeaponType::Unarmed, { 50, 120 } },
        { EWeaponType::Melee, { 50, 150 } },
        { EWeaponType::Pistol, { 500, 1800 } },
        { EWeaponType::Rifle, { 500, 1000 } },
    };
}

FString UBTDecorator_CheckTargetDistance::GetStaticDescription() const
{
    return FString::Printf(TEXT("Target: %s"), *TargetBlackboardKey.SelectedKeyName.ToString());
}

void UBTDecorator_CheckTargetDistance::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);

    ControlledCharacter = Cast<AShooterCharacter>(OwnerComp.GetAIOwner()->GetCharacter());
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!IsValid(Blackboard)) return;
    TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetBlackboardKey.SelectedKeyName));
}

void UBTDecorator_CheckTargetDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    if (!ControlledCharacter.IsValid()) return;
    if (!TargetActor.IsValid()) return;

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!IsValid(Blackboard)) return;

    UWeaponSlotsSystem* SlotsSystem = ControlledCharacter->GetWeaponSlotsSystem();
    if (!IsValid(SlotsSystem)) return;
    EWeaponType CurrentWeapon = ControlledCharacter->GetSelectedWeaponType();
    if (CurrentWeapon == EWeaponType::None) return;

    const FVector TargetLocation = TargetActor->GetActorLocation();
    const FVector ControlledLocation = ControlledCharacter->GetActorLocation();
    const float Distance = FVector::Distance(TargetLocation, ControlledLocation);

    FDistanceBounds* DistanceBounds = Distances.Find(CurrentWeapon);
    if (!DistanceBounds) DistanceBounds = &DefaultDistance;

    bool bNear = Distance < DistanceBounds->Lower;
    bool bFar = Distance > DistanceBounds->Upper;
    bool bOutOfAcceptedArea = bNear || bFar;

    ETargetDistance TargetDistance = ETargetDistance::None;
    if (bNear) TargetDistance = ETargetDistance::Near;
    else if (bFar) TargetDistance = ETargetDistance::Far;
    else if (!bOutOfAcceptedArea) TargetDistance = ETargetDistance::InAcceptedBounds;
    
    auto DistanceValue = static_cast<uint8>(TargetDistance);
    Blackboard->SetValueAsEnum(DistanceStateBlackboardKey.SelectedKeyName, DistanceValue);
}
