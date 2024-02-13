// Fill out your copyright notice in the Description page of Project Settings.


#include "Behavior/Services/BTService_SetShooterAimOffset.h"
#include "GameFramework/Character.h"
#include "Enemy/EnemyCharacter.h"


UBTService_SetShooterAimOffset::UBTService_SetShooterAimOffset(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bNotifyBecomeRelevant = true;
    NodeName = TEXT("Set Shooter Aim Offset");

    AimOffsetPitch = 0;
}

FString UBTService_SetShooterAimOffset::GetStaticDescription() const
{
    return FString::Printf(TEXT("Aim Offset: %.3f"), AimOffsetPitch);
}

void UBTService_SetShooterAimOffset::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);

    auto Character = Cast<AEnemyCharacter>(OwnerComp.GetAIOwner()->GetCharacter());
    if (IsValid(Character)) Character->SetAimOffsetPitch(AimOffsetPitch);
}

