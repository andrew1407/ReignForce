// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "LoadingScreenWidget.generated.h"


class UTexture2D;

/**
 * Fullscreen image with a loading indicator, shown while the next level is loaded in the background;
 * or an invisible screen cover that only takes the mouse input (a level restart keeps showing the game)
 */
UCLASS()
class REIGNFORCE_API ULoadingScreenWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetImage(UTexture2D* Image);

	/** Nothing is drawn (no background or indicator), the widget only blocks the clicks under it */
	UFUNCTION(BlueprintCallable)
	void SetInvisible(bool bInInvisible);

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsInvisible() const { return bInvisible; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY()
	TObjectPtr<UTexture2D> BackgroundImage;

	bool bBuilt = false;
	bool bInvisible = false;

	void Build();
	void ApplyInvisible();
};
