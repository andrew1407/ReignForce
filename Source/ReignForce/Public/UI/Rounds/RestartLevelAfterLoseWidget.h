// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RestartLevelAfterLoseWidget.generated.h"


class UTextBlock;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API URestartLevelAfterLoseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	URestartLevelAfterLoseWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure)
	FText GetCountdownDescriptionMessage() const;

	UFUNCTION(BlueprintCallable)
	void ResetCountdownMessage();

	UFUNCTION(BlueprintCallable)
	bool DecreaseCurrentSecondsCountdown(bool bUpdateWidget = true);

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsCountdownFull() const { return CountdownSecondsToRestart == CountdownSecondsLeft; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsNoSecondsLeft() const { return CountdownSecondsLeft <= 0; }

	FORCEINLINE void SetCountdownSecondsToRestart(int32 Seconds) { CountdownSecondsToRestart = FMath::Max(0, Seconds); }

	FORCEINLINE int32 GetCountdownSecondsToRestart() const { return CountdownSecondsToRestart; }
	FORCEINLINE int32 GetCountdownSecondsLeft() const { return CountdownSecondsLeft; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Round", meta = (BindWidget))
	TObjectPtr<UTextBlock> RestartMessage;	

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Seconds"))
	int32 CountdownSecondsToRestart;

	UPROPERTY(BlueprintReadOnly, Category = "Round", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Seconds"))
	int32 CountdownSecondsLeft;
};
