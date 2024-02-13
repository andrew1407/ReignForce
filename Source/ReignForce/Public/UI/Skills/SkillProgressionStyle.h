// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SkillProgressionStyle.generated.h"


USTRUCT(BlueprintType)
struct FSkillProgressionStyle
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> NormalImageState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> SelectedImageState;

	FORCEINLINE TArray<FSoftObjectPath> GetImageLoadInfo() const
	{
		return { NormalImageState.ToSoftObjectPath(), SelectedImageState.ToSoftObjectPath() };
	}

	FORCEINLINE bool IsImagesSet() const
	{
		return !(NormalImageState.IsNull() || NormalImageState.IsNull());
	}

	FORCEINLINE bool IsImagesLoaded() const
	{
		return NormalImageState.IsValid() && NormalImageState.IsValid();
	}

	FORCEINLINE TSoftObjectPtr<UTexture2D>& GetImageByState(bool bSelected)
	{
		return bSelected ? SelectedImageState : NormalImageState;
	}

	void Clear()
	{
		NormalImageState.Reset();
		SelectedImageState.Reset();
	}
};
