// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"


class UButton;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenuWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu", meta = (BindWidget))
	TObjectPtr<UButton> PlayButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu", meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu", meta = (AllowPrivateAccess = true))
	FName MainGameLevelName;

	UFUNCTION()
	void OnPlayClick();

	UFUNCTION()
	void OnExitClick();

};
