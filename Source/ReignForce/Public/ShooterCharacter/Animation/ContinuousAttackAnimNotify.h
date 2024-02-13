// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ContinuousAttackAnimNotify.generated.h"


class USkeletaMeshComponent;
class UAnimSequenceBase;

/**
 * 
 */
UCLASS(BlueprintType, meta = (DisplayName = "Continuous Attack"))
class REIGNFORCE_API UContinuousAttackAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	FORCEINLINE virtual FString GetNotifyName_Implementation() const override { return TEXT("Continuous Attack"); }
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
