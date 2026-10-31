// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Common/ReignForceActivatableWidget.h"
#include "MainMenuWidget.generated.h"


class UButton;
class UTextBlock;
class UConfirmationModalWidget;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UMainMenuWidget : public UReignForceActivatableWidget
{
	GENERATED_BODY()

public:
	UMainMenuWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable)
	void CloseCurrentOpenedModal();

	UFUNCTION(BlueprintPure)
	bool IsCurrentModalOpened() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual bool HandleBackAction() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu", meta = (BindWidget))
	TObjectPtr<UButton> ContinuePlayingButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu", meta = (BindWidget))
	TObjectPtr<UButton> PlayNewGameButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu", meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

	/** Created next to the exit button when the layout has no own one */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu", meta = (BindWidgetOptional))
	TObjectPtr<UButton> SettingsButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu", meta = (BindWidget))
	TObjectPtr<UTextBlock> ProjectVersion;

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
	void OnSettingsClick();

	UFUNCTION()
	void OnStartNewGameConfirmation(bool bAccepted);

	UFUNCTION()
	void OnExitConfirmation(bool bAccepted);

	void GracefulShutdown();
	bool GetHasLoadedSavesState() const;
	void GoToPlaygroundLevel();
	void OpenModalWindow(const TSubclassOf<UConfirmationModalWidget>& WidgetClass);
	void DefineCurrentProjectVersion();
};
