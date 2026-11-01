// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "StartRoundWidget.generated.h"


class UTextBlock;
class UImage;

/**
 * "Press T to start a new round." prompt: while a gamepad is used, the key is shown as the gamepad button icon instead
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

	/** True while the gamepad variant (button icon) of the prompt is shown */
	FORCEINLINE bool IsShowingGamepadPrompt() const { return bGamepadPrompt; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Round", meta = (BindWidget))
	TObjectPtr<UTextBlock> RoundStartMessage;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round", meta = (AllowPrivateAccess = true))
	FName StartActionKey;

	/** Gamepad button of the round start (reset round state) action in the gameplay mapping context */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round", meta = (AllowPrivateAccess = true))
	FKey StartActionGamepadKey;

	UPROPERTY()
	TObjectPtr<UImage> GamepadIcon;

	UPROPERTY()
	TObjectPtr<UTextBlock> GamepadSuffix;

	bool bGamepadPrompt = false;

	/** [message][icon][suffix] row in place of the message: the message is "Press" and the suffix the rest for the gamepad */
	void BuildGamepadPrompt();
	void SetGamepadPrompt(bool bGamepad);
	bool IsGamepadUsed() const;
};
