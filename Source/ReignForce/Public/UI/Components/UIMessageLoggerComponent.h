// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UIMessageLoggerComponent.generated.h"


class UMessageLoggerWidget;
class UGameMessageWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REIGNFORCE_API UUIMessageLoggerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UUIMessageLoggerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable)
	bool OpenGameMessageLogger();
	
	UFUNCTION(BlueprintPure)
	bool IsGameMessageLoggerOpened() const;

	UFUNCTION(BlueprintCallable)
	bool SetGameMessageLoggerVisibility(bool bVisible);

	UFUNCTION(BlueprintCallable)
	bool AddErrorLoggerMessageSavingDuringRound();

	UFUNCTION(BlueprintCallable)
	bool AddErrorLoggerMessageSavingOnPlayerDead();

	UFUNCTION(BlueprintCallable)
	bool AddLoggerMessageSavingSuccess();

protected:
	virtual void BeginDestroy() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MessageLogger", meta = (AllowPrivateAccess = true))
	TSubclassOf<UMessageLoggerWidget> MessageLoggerWidgetClass;
	
	UPROPERTY(BlueprintReadOnly, Category = "MessageLogger", meta = (AllowPrivateAccess = true))
	TObjectPtr<UMessageLoggerWidget> MessageLoggerWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MessageLogger", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Seconds"))
	float MessageCleanup;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MessageLogger", meta = (AllowPrivateAccess = true))
	int32 WidgetZOrder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessageLogger|Messages", meta = (AllowPrivateAccess = true))
	FText ErrorLoggerMessageSavingDuringRoundText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessageLogger|Messages", meta = (AllowPrivateAccess = true))
	FText ErrorLoggerMessageSavingOnPlayerDeadText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MessageLogger|Messages", meta = (AllowPrivateAccess = true))
	FText LoggerMessageSavingSuccessText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MessageLogger|Sounds", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> FailureMessageSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MessageLogger|Sounds", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> SuccessMessageSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MessageLogger|Sounds", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0, Units = "Times"))
	float MessageSoundVolume;

	UPROPERTY()
	FTimerHandle MessageCleanupTimerHandle;

	void SetCleanupMessageTimer(UGameMessageWidget* Message);
};
