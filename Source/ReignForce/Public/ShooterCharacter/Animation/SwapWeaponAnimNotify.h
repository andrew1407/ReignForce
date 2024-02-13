// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SwapWeaponAnimNotify.generated.h"


class USkeletaMeshComponent;
class UAnimSequenceBase;

/**
 * 
 */
UCLASS(BlueprintType, meta = (DisplayName = "Swap Weapon"))
class REIGNFORCE_API USwapWeaponAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	FORCEINLINE virtual FString GetNotifyName_Implementation() const override { return TEXT("Swap Weapon"); }

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
