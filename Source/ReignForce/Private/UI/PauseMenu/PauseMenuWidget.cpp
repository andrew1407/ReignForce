// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PauseMenu/PauseMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "GameStates/ShooterGameState.h"
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

    if (IsValid(SaveProgressButton)) SaveProgressButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSaveProgress);
    if (IsValid(RestartButton)) RestartButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnRestartClick);
    if (IsValid(ExitButton)) ExitButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnExitClick);
    if (IsValid(GoToMenuButton)) GoToMenuButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnGoToMenuClick);
    if (IsValid(SaveAndExitButton)) SaveAndExitButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSaveAndExit);

    if (IsValid(ContinueButton)) ContinueButton->SetFocus();
}

void UPauseMenuWidget::NativeDestruct()
{
    if (IsValid(ContinueButton)) ContinueButton->OnClicked.Clear();

    Super::NativeDestruct();
}

void UPauseMenuWidget::OnSaveProgress()
{
    auto Character = Cast<APlayerCharacter>(GetOwningPlayerPawn());
    if (IsValid(Character)) Character->SaveProgress();
}

void UPauseMenuWidget::OnRestartClick()
{
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

void UPauseMenuWidget::OnGoToMenuClick()
{
    if (MenuLevelName.IsNone()) return;

    if (bSaveOnGoToMenu) OnSaveProgress();

    constexpr bool bAbsolute = true;
	UGameplayStatics::OpenLevel(GetWorld(), MenuLevelName, bAbsolute);
}

void UPauseMenuWidget::OnExitClick()
{

    // Specify the quit preference (e.g., Quit, Restart, etc.)
    TEnumAsByte<EQuitPreference::Type> QuitPreference = EQuitPreference::Quit;

    // Specify whether to ignore platform restrictions
    constexpr bool bIgnorePlatformRestrictions = false;

    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), QuitPreference, bIgnorePlatformRestrictions);
}

void UPauseMenuWidget::OnSaveAndExit()
{
    OnSaveProgress();
    OnExitClick();
}
