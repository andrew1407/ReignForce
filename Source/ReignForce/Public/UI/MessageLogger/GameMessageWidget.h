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

	UFUNCTION(BlueprintCallable)
	bool SetDestroyTimeout(float Seconds = 1);

	UFUNCTION(BlueprintCallable)
	bool CancelDestroyTimeout();

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Message", meta = (BindWidget))
	TObjectPtr<UTextBlock> Message;

private:
	UPROPERTY()
	FTimerHandle CleanupTimer;
};
