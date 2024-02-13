// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/Decorators/BTDecorator_SetLookAtAimOffset.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"

#include "Components/SkeletalMeshComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "GameFramework/Controller.h"
#include "GameFramework/Character.h"

#include "Enemy/EnemyCharacter.h"


UBTDecorator_SetLookAtAimOffset::UBTDecorator_SetLookAtAimOffset(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bNotifyBecomeRelevant = true;
    bNotifyTick = true;

    NodeName = TEXT("Set Look At Aim Offset");
    TargetVisibilityBlackboardKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_SetLookAtAimOffset, TargetVisibilityBlackboardKey));
    TargetBlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_SetLookAtAimOffset, TargetBlackboardKey), AActor::StaticClass());

    LookFromSocketName = TEXT("head");
    LookAtSocketName = TEXT("head");
}

FString UBTDecorator_SetLookAtAimOffset::GetStaticDescription() const
{
    return FString::Printf(TEXT("Target: %s"), *TargetBlackboardKey.SelectedKeyName.ToString());
}

void UBTDecorator_SetLookAtAimOffset::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);

    OwnedCharacter = Cast<AEnemyCharacter>(OwnerComp.GetAIOwner()->GetCharacter());
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!IsValid(Blackboard)) return;
    TargetCharacter = Cast<ACharacter>(Blackboard->GetValueAsObject(TargetBlackboardKey.SelectedKeyName));
}

void UBTDecorator_SetLookAtAimOffset::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    if (!OwnedCharacter.IsValid()) return;

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!IsValid(Blackboard)) return;
    bool bTargetVisible = Blackboard->GetValueAsBool(TargetVisibilityBlackboardKey.SelectedKeyName);
    if (!bTargetVisible) return OwnedCharacter->SetAimOffsetPitch(0);

	USkeletalMeshComponent* ControlledMesh = OwnedCharacter->GetMesh();
    if (!IsValid(ControlledMesh)) return;

    if (!TargetCharacter.IsValid()) return;
	USkeletalMeshComponent* TargetMesh = TargetCharacter->GetMesh();
    if (!IsValid(TargetMesh)) return;

    const FTransform TargetTransform = TargetMesh->GetSocketTransform(LookAtSocketName, ERelativeTransformSpace::RTS_World);
    const FTransform ControlledTransform = ControlledMesh->GetSocketTransform(LookFromSocketName, ERelativeTransformSpace::RTS_World);

    const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(ControlledTransform.GetLocation(), TargetTransform.GetLocation());
    OwnedCharacter->SetAimOffsetPitch(LookAtRotation.Pitch);
}
