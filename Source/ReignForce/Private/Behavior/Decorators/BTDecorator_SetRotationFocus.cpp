// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/Decorators/BTDecorator_SetRotationFocus.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SkeletalMeshComponent.h"


UBTDecorator_SetRotationFocus::UBTDecorator_SetRotationFocus(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bNotifyBecomeRelevant = true;
    bNotifyTick = true;

    NodeName = TEXT("Set Rotation Focus");
    BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_SetRotationFocus, BlackboardKey), AActor::StaticClass());

    LookFromSocketName = TEXT("spine_03");
    LookAtSocketName = TEXT("spine_03");
}

FString UBTDecorator_SetRotationFocus::GetStaticDescription() const
{
    return FString::Printf(TEXT("Target: %s"), *BlackboardKey.SelectedKeyName.ToString());
}

void UBTDecorator_SetRotationFocus::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!IsValid(Blackboard)) return;
    Target = Cast<ACharacter>(Blackboard->GetValueAsObject(BlackboardKey.SelectedKeyName));
}

void UBTDecorator_SetRotationFocus::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    if (!Target.IsValid()) return;
    ACharacter* OwnedCharacter = OwnerComp.GetAIOwner()->GetCharacter();
    if (!IsValid(OwnedCharacter)) return;

    USkeletalMeshComponent* OwnedMesh = OwnedCharacter->GetMesh();
    if (!IsValid(OwnedMesh)) return;
    USkeletalMeshComponent* TargetMesh = Target->GetMesh();
    if (!IsValid(TargetMesh)) return;

    const FTransform TargetTransform = TargetMesh->GetSocketTransform(LookAtSocketName, ERelativeTransformSpace::RTS_World);
    const FTransform OwnedTransform = OwnedMesh->GetSocketTransform(LookFromSocketName, ERelativeTransformSpace::RTS_World);

    const FRotator PrevRotation = OwnedCharacter->GetActorRotation();
    FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(OwnedTransform.GetLocation(), TargetTransform.GetLocation());
    LookAtRotation.Pitch = PrevRotation.Pitch;
    OwnedCharacter->SetActorRotation(LookAtRotation);
}
