// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PauseMenu/PauseMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "GameModes/ReignForceGameMode.h"
#include "GameModes/Components/BackgroundMusicComponent.h"

#include "GameStates/ShooterGameState.h"
#include "UI/Modals/ConfirmationModalWidget.h"

#include "PlayerCharacter/PlayerCharacter.h"
#include "PlayerCharacter/ShooterPlayerController.h"
#include "PlayerCharacter/PlayerInteractionMode.h"


UPauseMenuWidget::UPauseMenuWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    MenuLevelName = NAME_None;
    bSaveOnGoToMenu = false;
}

void UPauseMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (IsValid(SaveProgressButton))
        SaveProgressButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSaveProgress);

    if (IsValid(RestartButton))
        RestartButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnRestartClick);

    if (IsValid(GoToMenuButton))
        GoToMenuButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnGoToMenuClick);

    if (IsValid(ExitButton))
        ExitButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnExitClick);

    if (IsValid(SaveAndExitButton))
        SaveAndExitButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSaveAndExit);

    if (IsValid(ContinueButton)) ContinueButton->SetFocus();

    auto GameMode = GetWorld()->GetAuthGameMode<AReignForceGameMode>();
    if (IsValid(GameMode))
    {
        UBackgroundMusicComponent* BackgroundMusic = GameMode->GetBackgroundMusicComponent();
        if (IsValid(BackgroundMusic)) BackgroundMusic->ResetActivePauseMenuSound();
    }
}

void UPauseMenuWidget::NativeDestruct()
{
    if (IsValid(ContinueButton)) ContinueButton->OnClicked.Clear();

    auto GameMode = GetWorld()->GetAuthGameMode<AReignForceGameMode>();
    if (IsValid(GameMode))
    {
        UBackgroundMusicComponent* BackgroundMusic = GameMode->GetBackgroundMusicComponent();
        if (IsValid(BackgroundMusic)) BackgroundMusic->StopActivePauseMenuSound();
    }

    Super::NativeDestruct();
}

void UPauseMenuWidget::OnSaveProgress()
{
    OpenModalWindow(SaveProgressConfirmationWidgetClass);
    if (!IsValid(CurrentOpenedModal)) return;

    FOnModalWindowWidgetActionResponse OnModalWindowWidgetActionResponse;
    OnModalWindowWidgetActionResponse.BindDynamic(this, &UPauseMenuWidget::SaveProgressConfirmation);
    CurrentOpenedModal->SetOnActionResponse(OnModalWindowWidgetActionResponse);
}

void UPauseMenuWidget::OnRestartClick()
{
    OpenModalWindow(RestartLevelConfirmationWidgetClass);
    if (!IsValid(CurrentOpenedModal)) return;

    FOnModalWindowWidgetActionResponse OnModalWindowWidgetActionResponse;
    OnModalWindowWidgetActionResponse.BindDynamic(this, &UPauseMenuWidget::RestartClickConfirmation);
    CurrentOpenedModal->SetOnActionResponse(OnModalWindowWidgetActionResponse);
}

void UPauseMenuWidget::OnGoToMenuClick()
{
    OpenModalWindow(GotToMenuConfirmationWidgetClass);
    if (!IsValid(CurrentOpenedModal)) return;

    FOnModalWindowWidgetActionResponse OnModalWindowWidgetActionResponse;
    OnModalWindowWidgetActionResponse.BindDynamic(this, &UPauseMenuWidget::GoToMenuClickConfirmation);
    CurrentOpenedModal->SetOnActionResponse(OnModalWindowWidgetActionResponse);
}

void UPauseMenuWidget::OnExitClick()
{
    OpenModalWindow(ExitConfirmationWidgetClass);
    if (!IsValid(CurrentOpenedModal)) return;

    FOnModalWindowWidgetActionResponse OnModalWindowWidgetActionResponse;
    OnModalWindowWidgetActionResponse.BindDynamic(this, &UPauseMenuWidget::ExitClickConfirmation);
    CurrentOpenedModal->SetOnActionResponse(OnModalWindowWidgetActionResponse);
}

void UPauseMenuWidget::OnSaveAndExit()
{
    OpenModalWindow(SaveAndExitConfirmationWidgetClass);
    if (!IsValid(CurrentOpenedModal)) return;

    FOnModalWindowWidgetActionResponse OnModalWindowWidgetActionResponse;
    OnModalWindowWidgetActionResponse.BindDynamic(this, &UPauseMenuWidget::SaveAndExitConfirmation);
    CurrentOpenedModal->SetOnActionResponse(OnModalWindowWidgetActionResponse);
}

void UPauseMenuWidget::SaveProgressConfirmation(bool bAccepted)
{
    if (bAccepted)
    {
        auto Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
        if (IsValid(Character)) Character->SaveProgress();
    }

    CloseCurrentOpenedModal();
    if (IsValid(SaveProgressButton)) SaveProgressButton->SetFocus();
}

void UPauseMenuWidget::RestartClickConfirmation(bool bAccepted)
{
    if (!bAccepted)
    {
        CloseCurrentOpenedModal();
        if (IsValid(RestartButton)) RestartButton->SetFocus();
        return;
    }

    APlayerController* PlayerController = GetOwningPlayer();
    if (IsValid(PlayerController))
    {
		auto Controller = GetOwningPlayer<AShooterPlayerController>();
        if (IsValid(Controller))
            Controller->SetInteractionMode(EPlayerInteractionMode::GameplayInput);
    }

    auto GameState = GetWorld()->GetGameState<AShooterGameState>();
    if (IsValid(GameState)) GameState->RestartLevel();
}

void UPauseMenuWidget::GoToMenuClickConfirmation(bool bAccepted)
{
    if (!bAccepted)
    {
        CloseCurrentOpenedModal();
        if (IsValid(GoToMenuButton)) GoToMenuButton->SetFocus();
        return;
    }

    if (MenuLevelName.IsNone()) return;
    if (bSaveOnGoToMenu) OnSaveProgress();
    constexpr bool bAbsolute = true;
	UGameplayStatics::OpenLevel(GetWorld(), MenuLevelName, bAbsolute);
}

void UPauseMenuWidget::ExitClickConfirmation(bool bAccepted)
{
    if (!bAccepted)
    {
        CloseCurrentOpenedModal();
        if (IsValid(ExitButton)) ExitButton->SetFocus();
        return;
    }

    // Specify the quit preference (e.g., Quit, Restart, etc.)
    TEnumAsByte<EQuitPreference::Type> QuitPreference = EQuitPreference::Quit;

    // Specify whether to ignore platform restrictions
    constexpr bool bIgnorePlatformRestrictions = false;

    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), QuitPreference, bIgnorePlatformRestrictions);
}

void UPauseMenuWidget::SaveAndExitConfirmation(bool bAccepted)
{
    if (!bAccepted)
    {
        CloseCurrentOpenedModal();
        if (IsValid(SaveAndExitButton)) SaveAndExitButton->SetFocus();
        return;
    }

    constexpr bool bConfirmed = true;
    SaveProgressConfirmation(bConfirmed);
    ExitClickConfirmation(bConfirmed);
}

void UPauseMenuWidget::CloseCurrentOpenedModal()
{
    if (!IsValid(CurrentOpenedModal)) return;
    CurrentOpenedModal->RemoveFromParent();
    CurrentOpenedModal = nullptr;
}

void UPauseMenuWidget::OpenModalWindow(const TSubclassOf<UConfirmationModalWidget>& WidgetClass)
{
    if (!IsValid(WidgetClass)) return;
    CloseCurrentOpenedModal();
    CurrentOpenedModal = CreateWidget<UConfirmationModalWidget>(GetWorld(), WidgetClass);
    if (IsValid(CurrentOpenedModal)) CurrentOpenedModal->AddToViewport();
}
