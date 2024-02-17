// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StartRoundWidget.generated.h"


class UTextBlock;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UStartRoundWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UStartRoundWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure)
	FText GetDescriptionMessage() const;

	UFUNCTION(BlueprintCallable)
	void UpdateRoundTextBlockDescrition();

	FORCEINLINE void SetStartActionKey(FName Key) { StartActionKey = Key; }

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Round", meta = (BindWidget))
	TObjectPtr<UTextBlock> RoundStartMessage;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round", meta = (AllowPrivateAccess = true))
	FName StartActionKey;
};
