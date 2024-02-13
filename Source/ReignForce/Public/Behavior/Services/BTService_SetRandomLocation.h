// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_SetRandomLocation.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API UBTService_SetRandomLocation : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_SetRandomLocation(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FString GetStaticDescription() const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector BlackboardKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	float SearchRadius;
};
