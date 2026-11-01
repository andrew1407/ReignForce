// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EditorAnimationUtils.generated.h"


class UBlendSpace;

/**
 * Editor only animation assets helpers exposed to editor scripting (Python), for edits the asset editors do on their own
 */
UCLASS()
class REIGNFORCE_API UEditorAnimationUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	/**
	 * Validates the samples & rebuilds the blend space runtime data (triangulation / grid weights) after its samples or axes
	 * were changed by a script: unlike the blend space editor, property changes alone don't rebuild it
	 */
	UFUNCTION(BlueprintCallable, Category = "Editor|Animation")
	static bool ResampleBlendSpace(UBlendSpace* BlendSpace);
#endif
};
