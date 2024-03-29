// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MessageLogger/GameMessageType.h"
#include "MessageLoggerWidget.generated.h"


class UPanelWidget;
class UGameMessageWidget;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UMessageLoggerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	UGameMessageWidget* AddMessageByType(const FText& Message, EGameMessageType MessageType, bool bClearBeforeAdd = false);

	UFUNCTION(BlueprintCallable)
	void ClearLog();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Messages", meta = (BindWidget))
	TObjectPtr<UPanelWidget> MessageContainer;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Messages", meta = (AllowPrivateAccess = true))
	TMap<EGameMessageType, TSubclassOf<UGameMessageWidget>> MessageTemplates;
};
