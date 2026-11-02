// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PauseMenu/PauseMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "GameModes/ReignForceGameMode.h"
#include "GameModes/LevelLoadingSubsystem.h"
#include "GameModes/Components/BackgroundMusicComponent.h"

#include "GameStates/ShooterGameState.h"
#include "UI/Modals/ConfirmationModalWidget.h"
#include "UI/Common/MenuLayoutUtils.h"

#include "PlayerCharacter/PlayerCharacter.h"
#include "PlayerCharacter/ShooterPlayerController.h"
#include "PlayerCharacter/PlayerInteractionMode.h"


UPauseMenuWidget::UPauseMenuWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    MenuLevelName = NAME_None;
    bSaveOnGoToMenu = false;
}

void UPauseMenuWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (!IsValid(SettingsButton))
    {
        constexpr int32 AfterRestart = 1;
        SettingsButton = MenuLayoutUtils::CreateButtonLike(WidgetTree, RestartButton, FText::FromString(TEXT("Settings")), TEXT("SettingsButton"), AfterRestart);
    }
    if (IsValid(SettingsButton))
        SettingsButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSettingsClick);

    // One more item than the layout was made for: tighter gaps, the column is centered & scrolls on short screens
    constexpr float ItemsGap = 16;
    MenuLayoutUtils::SetMenuItemsGap(ContinueButton, ItemsGap);
    // Keeps the column clear of the title at the top
    constexpr float ScreenMargin = 200;
    MakeMenuItemsScrollableCentered(ContinueButton, ScreenMargin);

    // Continue (and back action) asks the owner (HUD) to close the menu
    if (IsValid(ContinueButton))
        ContinueButton->OnClicked.AddDynamic(this, &UReignForceActivatableWidget::RequestClose);

    for (UButton* Button : { ContinueButton, SaveProgressButton, RestartButton, SettingsButton, GoToMenuButton, ExitButton, SaveAndExitButton })
        RegisterFocusableButton(Button);
    // Back action continues the game
    SetBackActionButton(ContinueButton);
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

    // The menu is created on every pause: the round & the player state don't change while it's open
    RefreshSaveAvailability();

    if (IsValid(ContinueButton)) FocusButton(ContinueButton);
}

void UPauseMenuWidget::RefreshSaveAvailability()
{
    auto Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    const bool bCanSave = IsValid(Character) && Character->CanSaveProgress();
    for (UButton* Button : { SaveProgressButton.Get(), SaveAndExitButton.Get() })
    {
        if (!IsValid(Button)) continue;
        Button->SetIsEnabled(bCanSave);
        // The menu buttons style has no disabled look of its own
        Button->SetRenderOpacity(bCanSave ? 1.f : .4f);
    }
    ChainMenuNavigation();
}

void UPauseMenuWidget::ChainMenuNavigation()
{
    UPanelWidget* Column = IsValid(ContinueButton) ? ContinueButton->GetParent() : nullptr;
    if (!IsValid(Column)) return;

    // The items in their column order (the settings item is inserted in the middle), only the enabled ones
    const TArray<UWidget*> Items = Column->GetAllChildren().FilterByPredicate([this] (const UWidget* Child)
    {
        return IsButtonAvailable(Cast<UButton>(Child));
    });
    constexpr bool bStopAfterLast = true;
    MenuLayoutUtils::ChainVerticalNavigation(Items, bStopAfterLast);
}

void UPauseMenuWidget::NativeDestruct()
{
    // The menu can be toggled off by the pause input while a modal is shown
    CloseCurrentOpenedModal();

    Super::NativeDestruct();
}

void UPauseMenuWidget::OnSettingsClick()
{
    OpenSettingsMenu(SettingsButton);
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
    if (IsValid(SaveProgressButton)) FocusButton(SaveProgressButton);
}

void UPauseMenuWidget::RestartClickConfirmation(bool bAccepted)
{
    if (!bAccepted)
    {
        CloseCurrentOpenedModal();
        if (IsValid(RestartButton)) FocusButton(RestartButton);
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
        if (IsValid(GoToMenuButton)) FocusButton(GoToMenuButton);
        return;
    }

    if (MenuLevelName.IsNone()) return;
    if (bSaveOnGoToMenu) OnSaveProgress();
    constexpr bool bAbsolute = true;
	// Level is streamed in behind the loading screen
	ULevelLoadingSubsystem::OpenLevel(this, MenuLevelName, bAbsolute);
}

void UPauseMenuWidget::ExitClickConfirmation(bool bAccepted)
{
    if (!bAccepted) return CloseCurrentOpenedModal();

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

void UPauseMenuWidget::SaveAndExitConfirmation(bool bAccepted)
{
    if (!bAccepted)
    {
        CloseCurrentOpenedModal();
        if (IsValid(SaveAndExitButton)) FocusButton(SaveAndExitButton);
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
    if (IsInViewport() && !IsActivated()) ActivateWidget();
}

void UPauseMenuWidget::OpenModalWindow(const TSubclassOf<UConfirmationModalWidget>& WidgetClass)
{
    if (!IsValid(WidgetClass)) return;
    CloseCurrentOpenedModal();
    CurrentOpenedModal = CreateWidget<UConfirmationModalWidget>(GetWorld(), WidgetClass);
    if (!IsValid(CurrentOpenedModal)) return;
    CurrentOpenedModal->AddToViewport();
    // The modal takes the input & focus over
    DeactivateWidget();
}
