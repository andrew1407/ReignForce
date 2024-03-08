// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"


class UButton;
class UConfirmationModalWidget;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPauseMenuWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	FORCEINLINE UButton* GetContinuePlayButton() const { return ContinueButton; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu", meta = (BindWidget))
	TObjectPtr<UButton> ContinueButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu", meta = (BindWidget))
	TObjectPtr<UButton> SaveProgressButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu", meta = (BindWidget))
	TObjectPtr<UButton> RestartButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu", meta = (BindWidget))
	TObjectPtr<UButton> GoToMenuButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu", meta = (BindWidget))
	TObjectPtr<UButton> SaveAndExitButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu", meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu", meta = (AllowPrivateAccess = true))
	FName MenuLevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu", meta = (AllowPrivateAccess = true))
	bool bSaveOnGoToMenu;

	#pragma region MODALS

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu|Modals", meta = (AllowPrivateAccess = true))
	TSubclassOf<UConfirmationModalWidget> SaveProgressConfirmationWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu|Modals", meta = (AllowPrivateAccess = true))
	TSubclassOf<UConfirmationModalWidget> RestartLevelConfirmationWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu|Modals", meta = (AllowPrivateAccess = true))
	TSubclassOf<UConfirmationModalWidget> GotToMenuConfirmationWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu|Modals", meta = (AllowPrivateAccess = true))
	TSubclassOf<UConfirmationModalWidget> SaveAndExitConfirmationWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu|Modals", meta = (AllowPrivateAccess = true))
	TSubclassOf<UConfirmationModalWidget> ExitConfirmationWidgetClass;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UConfirmationModalWidget> CurrentOpenedModal;

	#pragma endregion

	UFUNCTION()
	void OnSaveProgress();

	UFUNCTION()
	void OnRestartClick();

	UFUNCTION()
	void OnGoToMenuClick();

	UFUNCTION()
	void OnExitClick();

	UFUNCTION()
	void OnSaveAndExit();

	UFUNCTION()
	void SaveProgressConfirmation(bool bAccepted);

	UFUNCTION()
	void RestartClickConfirmation(bool bAccepted);

	UFUNCTION()
	void GoToMenuClickConfirmation(bool bAccepted);

	UFUNCTION()
	void ExitClickConfirmation(bool bAccepted);

	UFUNCTION()
	void SaveAndExitConfirmation(bool bAccepted);

	void CloseCurrentOpenedModal();
	void OpenModalWindow(const TSubclassOf<UConfirmationModalWidget>& WidgetClass);
};
