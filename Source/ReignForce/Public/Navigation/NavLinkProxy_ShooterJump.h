// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/NavLinkProxy.h"
#include "NavLinkProxy_ShooterJump.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API ANavLinkProxy_ShooterJump : public ANavLinkProxy
{
	GENERATED_BODY()

public:
	ANavLinkProxy_ShooterJump(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	float JumpDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (AllowPrivateAccess = true))
	bool bActivateJumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, ClampMax = 1, UIMax = 1, EditCondition = "bActivateJumpAction", EditConditionHides))
	float JumpActionProbability;

	UFUNCTION()
	void HandleSmartLinkReached(AActor* Agent, const FVector& Destination);
};
