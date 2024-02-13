// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/Services/BTService_SetSpeed.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"


UBTService_SetSpeed::UBTService_SetSpeed(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bNotifyBecomeRelevant = true;
    NodeName = TEXT("Set Speed");

    Speed = 0;
}

FString UBTService_SetSpeed::GetStaticDescription() const
{
    return FString::Printf(TEXT("Speed: %.3f"), Speed);
}

void UBTService_SetSpeed::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);

    ACharacter* Character = OwnerComp.GetAIOwner()->GetCharacter();
    if (!IsValid(Character)) return;
    UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    if (!IsValid(Movement)) return;
    Movement->MaxWalkSpeed = Speed;
}
