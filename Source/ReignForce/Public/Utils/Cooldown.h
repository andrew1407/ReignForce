// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Cooldown.generated.h"


/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class REIGNFORCE_API UCooldown : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	FORCEINLINE bool HasCooldown(UObject* Target) const { return ActiveCooldowns.Contains(Target); }

	UFUNCTION(BlueprintCallable)
	bool Activate(UObject* Target, float Duration, bool bApplyRefrash = false);

	UFUNCTION(BlueprintCallable)
	bool Refresh(UObject* Target, float Duration);

private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	TMap<UObject*, FTimerHandle> ActiveCooldowns;
};
