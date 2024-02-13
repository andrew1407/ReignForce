// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_SetSpeed.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API UBTService_SetSpeed : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_SetSpeed(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FString GetStaticDescription() const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OprionsToSet", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	float Speed;
};
