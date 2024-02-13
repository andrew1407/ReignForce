// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AimWidget.generated.h"


class UImage;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UAimWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetAimVisibility(bool bState);

	UFUNCTION(BlueprintCallable)
	void SetAimTargetDetected(bool bState);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aim", meta = (BindWidget))
	TObjectPtr<UImage> AimImage;
};
