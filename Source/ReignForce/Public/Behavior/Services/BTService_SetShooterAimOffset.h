// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_SetShooterAimOffset.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API UBTService_SetShooterAimOffset : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_SetShooterAimOffset(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FString GetStaticDescription() const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OprionsToSet", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, ClampMax = 360, UIMax = 360, Units = "Degrees"))
	float AimOffsetPitch;
};
