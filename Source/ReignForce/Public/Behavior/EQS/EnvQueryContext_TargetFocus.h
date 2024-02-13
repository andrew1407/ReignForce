// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_TargetFocus.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API UEnvQueryContext_TargetFocus : public UEnvQueryContext
{
	GENERATED_BODY()

public:
	UEnvQueryContext_TargetFocus(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (AllowPrivateAccess = true))
    FName TargetBlackboardKey;
};
