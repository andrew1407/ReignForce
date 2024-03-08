// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MainMenu/MainMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "UI/Modals/ConfirmationModalWidget.h"
#include "GameStates/ShooterGameState.h"
#include "GameModes/ReignForceGameMode.h"
#include "GameModes/Components/BackgroundMusicComponent.h"


UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    MainGameLevelName = NAME_None;
}

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    bool bPlayerHasLoadedSavesState = GetHasLoadedSavesState();
    if (IsValid(ContinuePlayingButton))
    {
        ContinuePlayingButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnContinuePlayingClick);
        ContinuePlayingButton->SetIsEnabled(bPlayerHasLoadedSavesState);
        if (bPlayerHasLoadedSavesState) ContinuePlayingButton->SetFocus();
    }

    if (IsValid(PlayNewGameButton))
    {
        PlayNewGameButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayNewGameClick);
        if (!bPlayerHasLoadedSavesState) PlayNewGameButton->SetFocus();
    }

    if (IsValid(ExitButton))
        ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnExitClick);
}

void UMainMenuWidget::CloseCurrentOpenedModal()
{
    if (!IsCurrentModalOpened()) return;
    CurrentOpenedModal->RemoveFromParent();
    CurrentOpenedModal = nullptr;
}

bool UMainMenuWidget::IsCurrentModalOpened() const
{
    return IsValid(CurrentOpenedModal);
}

void UMainMenuWidget::OnContinuePlayingClick()
{
    GoToPlaygroundLevel();
}

void UMainMenuWidget::OnPlayNewGameClick()
{
    bool bPlayerHasLoadedSavesState = GetHasLoadedSavesState();
    if (!bPlayerHasLoadedSavesState) return GoToPlaygroundLevel();
    if (!IsValid(NewGameConfirmationModalWidgetClass)) return;

    OpenModalWindow(NewGameConfirmationModalWidgetClass);
    if (!IsCurrentModalOpened()) return;

    FOnModalWindowWidgetActionResponse OnModalWindowWidgetActionResponse;
    OnModalWindowWidgetActionResponse.BindDynamic(this, &UMainMenuWidget::OnStartNewGameConfirmation);
    CurrentOpenedModal->SetOnActionResponse(OnModalWindowWidgetActionResponse);
}

void UMainMenuWidget::OnExitClick()
{
    if (!IsValid(ExitUConfirmationModalWidgetClass)) return GracefulShutdown();

    OpenModalWindow(ExitUConfirmationModalWidgetClass);
    if (!IsCurrentModalOpened()) return;

    FOnModalWindowWidgetActionResponse OnModalWindowWidgetActionResponse;
    OnModalWindowWidgetActionResponse.BindDynamic(this, &UMainMenuWidget::OnExitConfirmation);
    CurrentOpenedModal->SetOnActionResponse(OnModalWindowWidgetActionResponse);
}

void UMainMenuWidget::OnStartNewGameConfirmation(bool bAccepted)
{
    if (bAccepted)
    {
        auto GameState = GetWorld()->GetGameState<AShooterGameState>();
        if (IsValid(GameState)) GameState->ClearPlayerCharacterState();
        GoToPlaygroundLevel();
    }
    else
    {
        CloseCurrentOpenedModal();
        if (IsValid(PlayNewGameButton)) PlayNewGameButton->SetFocus();
    }
}

void UMainMenuWidget::OnExitConfirmation(bool bAccepted)
{
    if (bAccepted) return GracefulShutdown();
    CloseCurrentOpenedModal();
    if (IsValid(ExitButton)) ExitButton->SetFocus();
}

void UMainMenuWidget::GracefulShutdown()
{
    auto GameMode = GetWorld()->GetAuthGameMode<AReignForceGameMode>();
    if (IsValid(GameMode))
    {
        UBackgroundMusicComponent* BackgroundMusic = GameMode->GetBackgroundMusicComponent();
        if (IsValid(BackgroundMusic)) BackgroundMusic->StopActiveMainMenuSound();
    }

    // Specify the quit preference (e.g., Quit, Restart, etc.)
    TEnumAsByte<EQuitPreference::Type> QuitPreference = EQuitPreference::Quit;

    // Specify whether to ignore platform restrictions
    constexpr bool bIgnorePlatformRestrictions = false;

    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), QuitPreference, bIgnorePlatformRestrictions);
}

bool UMainMenuWidget::GetHasLoadedSavesState() const
{
    auto GameState = GetWorld()->GetGameState<AShooterGameState>();
    return IsValid(GameState) && GameState->DoesPlayerHaveSavedProgress();
}

void UMainMenuWidget::GoToPlaygroundLevel()
{
    if (MainGameLevelName.IsNone()) return;
    constexpr bool bAbsolute = true;
	UGameplayStatics::OpenLevel(GetWorld(), MainGameLevelName, bAbsolute);
}

void UMainMenuWidget::OpenModalWindow(const TSubclassOf<UConfirmationModalWidget>& WidgetClass)
{
    if (!IsValid(WidgetClass)) return;
    CloseCurrentOpenedModal();
    CurrentOpenedModal = CreateWidget<UConfirmationModalWidget>(GetWorld(), WidgetClass);
    if (IsValid(CurrentOpenedModal)) CurrentOpenedModal->AddToViewport();
}
