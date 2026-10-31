// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MainMenu/MainMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Modules/ModuleManager.h"
#include "Misc/ConfigCacheIni.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "UI/Modals/ConfirmationModalWidget.h"
#include "GameStates/ShooterGameState.h"
#include "GameModes/ReignForceGameMode.h"
#include "GameModes/LevelLoadingSubsystem.h"
#include "GameModes/Components/BackgroundMusicComponent.h"


UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    MainGameLevelName = NAME_None;
}

void UMainMenuWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (!IsValid(SettingsButton))
        SettingsButton = CreateButtonLike(ExitButton, FText::FromString(TEXT("Settings")), TEXT("SettingsButton"));
    if (IsValid(SettingsButton))
        SettingsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsClick);

    RegisterFocusableButton(ContinuePlayingButton);
    RegisterFocusableButton(PlayNewGameButton);
    RegisterFocusableButton(SettingsButton);
    RegisterFocusableButton(ExitButton);
}

bool UMainMenuWidget::HandleBackAction()
{
    // Back on the main menu asks to quit the game
    if (IsCurrentModalOpened()) return false;
    OnExitClick();
    return true;
}

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    DefineCurrentProjectVersion();

    bool bPlayerHasLoadedSavesState = GetHasLoadedSavesState();
    if (IsValid(ContinuePlayingButton))
    {
        ContinuePlayingButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnContinuePlayingClick);
        ContinuePlayingButton->SetIsEnabled(bPlayerHasLoadedSavesState);
        if (bPlayerHasLoadedSavesState) FocusButton(ContinuePlayingButton);
    }

    if (IsValid(PlayNewGameButton))
    {
        PlayNewGameButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayNewGameClick);
        if (!bPlayerHasLoadedSavesState) FocusButton(PlayNewGameButton);
    }

    if (IsValid(ExitButton))
        ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnExitClick);
}


void UMainMenuWidget::NativeDestruct()
{
    CloseCurrentOpenedModal();

    auto GameMode = GetWorld()->GetAuthGameMode<AReignForceGameMode>();
    if (IsValid(GameMode))
    {
        UBackgroundMusicComponent* BackgroundMusic = GameMode->GetBackgroundMusicComponent();
        if (IsValid(BackgroundMusic)) BackgroundMusic->StopAllActiveMusic();
    }

    Super::NativeDestruct();
}

void UMainMenuWidget::CloseCurrentOpenedModal()
{
    if (!IsCurrentModalOpened()) return;
    CurrentOpenedModal->RemoveFromParent();
    CurrentOpenedModal = nullptr;
    if (IsInViewport() && !IsActivated()) ActivateWidget();
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

void UMainMenuWidget::OnSettingsClick()
{
    OpenSettingsMenu(SettingsButton);
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
        if (IsValid(PlayNewGameButton)) FocusButton(PlayNewGameButton);
    }
}

void UMainMenuWidget::OnExitConfirmation(bool bAccepted)
{
    if (bAccepted) return GracefulShutdown();
    CloseCurrentOpenedModal();
    if (IsValid(ExitButton)) FocusButton(ExitButton);
}

void UMainMenuWidget::GracefulShutdown()
{
    auto GameMode = GetWorld()->GetAuthGameMode<AReignForceGameMode>();
    if (IsValid(GameMode))
    {
        UBackgroundMusicComponent* BackgroundMusic = GameMode->GetBackgroundMusicComponent();
        if (IsValid(BackgroundMusic)) BackgroundMusic->StopAllActiveMusic();
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
	// Level is streamed in behind the loading screen
	ULevelLoadingSubsystem::OpenLevel(this, MainGameLevelName, bAbsolute);
}

void UMainMenuWidget::OpenModalWindow(const TSubclassOf<UConfirmationModalWidget>& WidgetClass)
{
    if (!IsValid(WidgetClass)) return;
    CloseCurrentOpenedModal();
    CurrentOpenedModal = CreateWidget<UConfirmationModalWidget>(GetWorld(), WidgetClass);
    if (!IsValid(CurrentOpenedModal)) return;
    CurrentOpenedModal->AddToViewport();
    // The modal takes the input & focus over
    DeactivateWidget();
}

void UMainMenuWidget::DefineCurrentProjectVersion()
{
    if (!IsValid(ProjectVersion)) return;

    FString Version;
    GConfig->GetString(
        TEXT("/Script/EngineSettings.GeneralProjectSettings"),
        TEXT("ProjectVersion"),
        Version,
        GGameIni
    );
    
    const FText TextContent = FText::FromString(TEXT("v") + Version);
    ProjectVersion->SetText(TextContent);
}