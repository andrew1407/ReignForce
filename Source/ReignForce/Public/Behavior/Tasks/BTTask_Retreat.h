// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Retreat.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API UBTTask_Retreat : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Retreat(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FString GetStaticDescription() const override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	EBTNodeResult::Type FinishWithResult(UBehaviorTreeComponent& OwnerComp, EBTNodeResult::Type Type);

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector BlackboardKey;

	EBTNodeResult::Type Strafe(UBehaviorTreeComponent& OwnerComp);
};
