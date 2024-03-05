// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"


class UButton;
class UConfirmationModalWidget;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenuWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable)
	void CloseCurrentOpenedModal();

	UFUNCTION(BlueprintPure)
	bool IsCurrentModalOpened() const;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu", meta = (BindWidget))
	TObjectPtr<UButton> ContinuePlayingButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu", meta = (BindWidget))
	TObjectPtr<UButton> PlayNewGameButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu", meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu", meta = (AllowPrivateAccess = true))
	FName MainGameLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu|Modals", meta = (AllowPrivateAccess = true))
	TSubclassOf<UConfirmationModalWidget> NewGameConfirmationModalWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu|Modals", meta = (AllowPrivateAccess = true))
	TSubclassOf<UConfirmationModalWidget> ExitUConfirmationModalWidgetClass;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UConfirmationModalWidget> CurrentOpenedModal;

	UFUNCTION()
	void OnContinuePlayingClick();

	UFUNCTION()
	void OnPlayNewGameClick();

	UFUNCTION()
	void OnExitClick();

	UFUNCTION()
	void OnStartNewGameConfirmation(bool bAccepted);

	UFUNCTION()
	void OnExitConfirmation(bool bAccepted);

	void GracefulShutdown();
	bool GetHasLoadedSavesState() const;
	void GoToPlaygroundLevel();
	void OpenModalWindow(const TSubclassOf<UConfirmationModalWidget>& WidgetClass);
};
