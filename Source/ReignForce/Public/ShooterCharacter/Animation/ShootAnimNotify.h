// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ShootAnimNotify.generated.h"


class USkeletaMeshComponent;
class UAnimSequenceBase;

/**
 * 
 */
UCLASS(BlueprintType, meta = (DisplayName = "Shoot With Weapon"))
class REIGNFORCE_API UShootAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	FORCEINLINE virtual FString GetNotifyName_Implementation() const override { return TEXT("Shoot With Weapon"); }

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
