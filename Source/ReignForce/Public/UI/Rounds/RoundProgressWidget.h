// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RoundProgressWidget.generated.h"


class UTextBlock;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API URoundProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetEnemiesToKillTotalCount(int32 Count);

	UFUNCTION(BlueprintCallable)
	void SetEnemiesToKillCurrentCount(int32 Count);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Round", meta = (BindWidget))
	TObjectPtr<UTextBlock> EnemiesToKillTotal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Round", meta = (BindWidget))
	TObjectPtr<UTextBlock> EnemiesToKillCurrent;
};
