// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/Tasks/BTTask_Retreat.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "ShooterCharacter/ShooterCharacter.h"
#include "Weapons/Components/WeaponSlotsSystem.h"


UBTTask_Retreat::UBTTask_Retreat(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bNotifyTick = true;
    NodeName = TEXT("Retreat");
    BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Retreat, BlackboardKey));

}

FString UBTTask_Retreat::GetStaticDescription() const
{
    return FString::Printf(TEXT("To: %s"), *BlackboardKey.SelectedKeyName.ToString());
}

EBTNodeResult::Type UBTTask_Retreat::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    return Strafe(OwnerComp);
}

void UBTTask_Retreat::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    Strafe(OwnerComp);
}

EBTNodeResult::Type UBTTask_Retreat::Strafe(UBehaviorTreeComponent& OwnerComp)
{
    ACharacter* Character = OwnerComp.GetAIOwner()->GetCharacter();
    if (!IsValid(Character)) return FinishWithResult(OwnerComp, EBTNodeResult::Type::Failed);
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!IsValid(Blackboard)) return FinishWithResult(OwnerComp, EBTNodeResult::Type::Failed);
    bool IsStrafeLocationSet = Blackboard->IsVectorValueSet(BlackboardKey.SelectedKeyName);
    if (!IsStrafeLocationSet) return FinishWithResult(OwnerComp, EBTNodeResult::Type::Failed);
    
    const FVector CharacterLocation = Character->GetActorLocation();
    const FVector RetreatLocation = Blackboard->GetValueAsVector(BlackboardKey.SelectedKeyName);
    const float Distance = FVector::Dist(RetreatLocation, CharacterLocation);

    bool IsLocationReached = FMath::IsNearlyZero(Distance);
    if (IsLocationReached) return FinishWithResult(OwnerComp, EBTNodeResult::Type::Succeeded);
    
    const FVector Direction = (RetreatLocation - CharacterLocation).GetSafeNormal();
    Character->AddMovementInput(Direction);
    return FinishWithResult(OwnerComp, EBTNodeResult::Type::InProgress);
}

EBTNodeResult::Type UBTTask_Retreat::FinishWithResult(UBehaviorTreeComponent& OwnerComp, EBTNodeResult::Type Type)
{
    bool bFinishType = Type == EBTNodeResult::Type::Failed || Type == EBTNodeResult::Type::Succeeded;
    if (bFinishType) FinishLatentTask(OwnerComp, Type);
    return Type;
}
