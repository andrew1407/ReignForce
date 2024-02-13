// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Weapons/ShooterState.h"
#include "ShooterStateAnimNotify.generated.h"


class USkeletaMeshComponent;
class UAnimSequenceBase;

/**
 * 
 */
UCLASS(BlueprintType, meta = (DisplayName = "Set Shooter State"))
class REIGNFORCE_API UShooterStateAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	UShooterStateAnimNotify(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	FORCEINLINE virtual FString GetNotifyName_Implementation() const override { return TEXT("Set Shooter State"); }

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true))
	EShooterState ShooterState;
};
