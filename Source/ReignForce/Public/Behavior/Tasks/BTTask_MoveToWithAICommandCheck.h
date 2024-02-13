// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "ReinforcementLearning/ShooterAICommandCheck.h"
#include "BTTask_MoveToWithAICommandCheck.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API UBTTask_MoveToWithAICommandCheck : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UBTTask_MoveToWithAICommandCheck(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector ShooterAICommandBlackboardKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AICommand", meta = (AllowPrivateAccess = true))
	FShooterAICommandCheck ShooterAICommandCheck;
};
