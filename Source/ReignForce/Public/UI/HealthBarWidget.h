// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"


class UProgressBar;
class UTextBlock;
class UCurveLinearColor;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetHealthValue(int32 Value, float Percent);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (BindWidget))
	TObjectPtr<UTextBlock> NumericValue;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCurveLinearColor> HealthBarColorCurve;
};
