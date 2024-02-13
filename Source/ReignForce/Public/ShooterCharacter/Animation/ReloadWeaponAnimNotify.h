// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ReloadWeaponAnimNotify.generated.h"


class USkeletaMeshComponent;
class UAnimSequenceBase;

/**
 * 
 */
UCLASS(BlueprintType, meta = (DisplayName = "Reload Weapon"))
class REIGNFORCE_API UReloadWeaponAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	FORCEINLINE virtual FString GetNotifyName_Implementation() const override { return TEXT("Reload Weapon"); }

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
