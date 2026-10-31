// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "UI/Common/ReignForceActivatableWidget.h"
#include "GamepadSchemeWidget.generated.h"


class UButton;
class UCanvasPanel;
class UTexture2D;

/** Gamepad control action pointed on the scheme image */
USTRUCT(BlueprintType)
struct FGamepadSchemeCallout
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FKey Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Label;

	/** Control position on the scheme image (0..1 of its size) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Anchor = FVector2D::ZeroVector;

	/** Label vertical position (0..1 of the scheme height), labels are placed in columns at the image sides */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, ClampMax = 1, UIMax = 1))
	float LabelHeight = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLeftSide = true;
};

/**
 * Gamepad controls scheme: controller image with the actions labels connected to its buttons & sticks by leader lines
 */
UCLASS()
class REIGNFORCE_API UGamepadSchemeWidget : public UReignForceActivatableWidget
{
	GENERATED_BODY()

public:
	UGamepadSchemeWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Builds the menu taking buttons style from the template (should be called before adding to the viewport) */
	UFUNCTION(BlueprintCallable)
	void Setup(UButton* StyleTemplate);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scheme")
	TArray<FGamepadSchemeCallout> Callouts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scheme|Layout", meta = (ClampMin = 0, UIMin = 0))
	FVector2D SchemeSize;

	/** Controller image width, the height keeps the texture aspect ratio */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scheme|Layout", meta = (ClampMin = 0, UIMin = 0))
	float ImageWidth;

	/** Gap between the image and the labels columns */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scheme|Layout", meta = (ClampMin = 0, UIMin = 0))
	float LabelsGap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scheme|Layout")
	FLinearColor BackgroundColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scheme|Layout")
	FLinearColor LineColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scheme|Layout")
	FLinearColor AnchorColor;

private:
	bool bBuilt;
	FVector2D ImageSize;

	UPROPERTY()
	TObjectPtr<UCanvasPanel> Scheme;

	UPROPERTY()
	TObjectPtr<UButton> BackButton;

	FVector2D GetImageOrigin() const;
	FVector2D GetLabelEdge(const FGamepadSchemeCallout& Callout) const;
};
