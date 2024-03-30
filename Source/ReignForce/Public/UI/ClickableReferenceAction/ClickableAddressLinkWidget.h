// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ClickableReferenceAction/ClickableReferenceActionWidget.h"
#include "ClickableAddressLinkWidget.generated.h"

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UClickableAddressLinkWidget : public UClickableReferenceActionWidget
{
	GENERATED_BODY()

public:
    virtual void OnActionReference_Implementation() override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AddressLink", meta = (AllowPrivateAccess = true))
	FString AddressLink;
};
