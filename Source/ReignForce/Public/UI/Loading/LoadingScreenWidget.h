// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "LoadingScreenWidget.generated.h"


class UTexture2D;

/**
 * Fullscreen image with a loading indicator, shown while the next level is loaded in the background
 */
UCLASS()
class REIGNFORCE_API ULoadingScreenWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetImage(UTexture2D* Image);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY()
	TObjectPtr<UTexture2D> BackgroundImage;

	bool bBuilt = false;

	void Build();
};
