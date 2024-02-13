// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MakeNoiseAnimNotify.generated.h"


class USkeletaMeshComponent;
class UAnimSequenceBase;

/**
 * 
 */
UCLASS(BlueprintType, meta = (DisplayName = "Make Noise"))
class REIGNFORCE_API UMakeNoiseAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	UMakeNoiseAnimNotify(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	FORCEINLINE virtual FString GetNotifyName_Implementation() const override { return TEXT("Make Noise"); }

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise", meta = (AllowPrivateAccess = true))
	FName NoiseTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Times"))
	float Loudness;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	float DefaultLoudnessRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise", meta = (AllowPrivateAccess = true))
	FName MeshSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> SoundToPlay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = true))
	float SoundToPlayVolume;

	TPair<FVector, FRotator> GetNoisePosition(USkeletalMeshComponent* MeshComp) const;
};
