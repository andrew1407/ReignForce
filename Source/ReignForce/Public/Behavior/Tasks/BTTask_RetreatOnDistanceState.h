// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Behavior/Tasks/BTTask_Retreat.h"
#include "ReinforcementLearning/TargetDistance.h"
#include "BTTask_RetreatOnDistanceState.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API UBTTask_RetreatOnDistanceState : public UBTTask_Retreat
{
	GENERATED_BODY()

public:
	UBTTask_RetreatOnDistanceState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector DistanceStateBlackboardKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance", meta = (AllowPrivateAccess = true))
	ETargetDistance ActivityState;

	EBTNodeResult::Type CheckState(UBehaviorTreeComponent& OwnerComp);
};
