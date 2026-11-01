// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FocusOutline.generated.h"


class UWidget;
class FSlateWindowElementList;

/**
 * Translucent frame drawn around the focused widget, outside its bounds (no layout, scale or style changes)
 */
USTRUCT(BlueprintType)
struct REIGNFORCE_API FFocusOutlineStyle
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus")
	FLinearColor Color = FLinearColor(1, 1, 1, .6f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus", meta = (ClampMin = 0, UIMin = 0))
	float Thickness = 4;

	/** Space between the widget edge and the frame */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus", meta = (ClampMin = 0, UIMin = 0))
	float Gap = 2;
};

namespace FocusOutline
{
	/**
	 * Paints the frame around the target by its geometry painted this frame (should be called after the target is painted),
	 * clipped by the nearest scroll box the target is placed in. Returns the layer the frame is painted on
	 */
	REIGNFORCE_API int32 Paint(const UWidget* Target, const FFocusOutlineStyle& Style, FSlateWindowElementList& OutDrawElements, int32 LayerId);
}
