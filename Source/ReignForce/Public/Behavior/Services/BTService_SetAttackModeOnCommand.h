// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "ReinforcementLearning/ShooterAICommandCheck.h"
#include "BTService_SetAttackModeOnCommand.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API UBTService_SetAttackModeOnCommand : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_SetAttackModeOnCommand(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FString GetStaticDescription() const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector BlackboardKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AICommand", meta = (AllowPrivateAccess = true))
	FShooterAICommandCheck ShooterAICommandCheck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AICommand", meta = (AllowPrivateAccess = true))
	bool bMode;
};
