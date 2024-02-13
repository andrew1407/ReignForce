// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "ReinforcementLearning/TargetDistance.h"
#include "BTTask_MoveToOnDistanceState.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API UBTTask_MoveToOnDistanceState : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UBTTask_MoveToOnDistanceState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector DistanceStateBlackboardKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (AllowPrivateAccess = true))
	ETargetDistance ActivityState;
};
