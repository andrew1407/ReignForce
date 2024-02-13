// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PlayAttackGruntAnimNotify.generated.h"


class USkeletaMeshComponent;
class UAnimSequenceBase;

/**
 * 
 */
UCLASS(BlueprintType, meta = (DisplayName = "Play Attack Grunt"))
class REIGNFORCE_API UPlayAttackGruntAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	FORCEINLINE virtual FString GetNotifyName_Implementation() const override { return TEXT("Play Attack Grunt"); }

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
