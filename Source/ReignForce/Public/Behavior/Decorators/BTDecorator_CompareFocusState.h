// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "ReinforcementLearning/TargetDistanceCheck.h"
#include "BTDecorator_CompareFocusState.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API UBTDecorator_CompareFocusState : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CompareFocusState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FString GetStaticDescription() const override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State", meta = (AllowPrivateAccess = true))
	FTargetDistanceCheck TargetDistanceCheck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector BlackboardKey;
};
