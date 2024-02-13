// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ApplyMeleeAnimNotifyState.generated.h"


class USkeletaMeshComponent;
class UAnimSequenceBase;

/**
 * 
 */
UCLASS(BlueprintType, meta = (DisplayName = "Apply Melee"))
class REIGNFORCE_API UApplyMeleeAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UApplyMeleeAnimNotifyState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	FORCEINLINE virtual FString GetNotifyName_Implementation() const override { return TEXT("Apply Melee"); }

	void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration);
	void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = true))
	int32 CollisionParam;

	void SetCollisionState(AActor* Actor, bool bState);
};
