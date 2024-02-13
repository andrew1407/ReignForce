// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Behavior/Tasks/BTTask_Retreat.h"
#include "ReinforcementLearning/ShooterAICommandCheck.h"
#include "BTTask_RetreatWithAICommandCheck.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API UBTTask_RetreatWithAICommandCheck : public UBTTask_Retreat
{
	GENERATED_BODY()

public:
	UBTTask_RetreatWithAICommandCheck(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector ShooterAICommandBlackboardKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AICommand", meta = (AllowPrivateAccess = true))
	FShooterAICommandCheck ShooterAICommandCheck;

	EBTNodeResult::Type CheckState(UBehaviorTreeComponent& OwnerComp);
};
