// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponItemWidget.generated.h"


class UImage;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UWeaponItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWeaponItemWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable)
	virtual void SetWeaponSelected(bool bState);

	UFUNCTION(BlueprintCallable)
	virtual void SetItemVisible(bool bState);

	FORCEINLINE UImage* GetWeaponIcon() const { return WeaponIcon; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Icon", meta = (BindWidget))
	TObjectPtr<UImage> WeaponIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Icon", meta = (ClampMin = 0, UIMin = 0, ClampMax = 1, UIMax = 1))
	float InactiveOpacity;
};
