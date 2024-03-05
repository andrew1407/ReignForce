// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameMessageWidget.generated.h"


class UTextBlock;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UGameMessageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetMessageText(const FText& ToSet);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Message", meta = (BindWidget))
	TObjectPtr<UTextBlock> Message;
};
