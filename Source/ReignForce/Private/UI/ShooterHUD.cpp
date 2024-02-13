// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ShooterHUD.h"
#include "Components/Button.h"

#include "GameModes/ReignForceGameMode.h"
#include "GameModes/Components/BackgroundMusicComponent.h"

#include "GameStates/ShooterGameState.h"
#include "GameStates/Components/UpgradesProgressStateComponent.h"

#include "PlayerCharacter/ShooterPlayerController.h"
#include "PlayerCharacter/PlayerInteractionMode.h"

#include "UI/AimWidget.h"
#include "UI/ShooterStatsWidget.h"
#include "UI/Skills/SkillsProgressionWidget.h"
#include "UI/PauseMenu/PauseMenuWidget.h"


void AShooterHUD::BeginPlay()
{
    Super::BeginPlay();

	auto GameState = GetWorld()->GetGameState<AShooterGameState>();
    if (IsValid(GameState))
    {
        UUpgradesProgressStateComponent* UpgradesProgressStateComponent = GameState->GetUpgradesProgressStateComponent();
        if (IsValid(UpgradesProgressStateComponent))
        {
            UpgradesProgressStateComponent->OnSkillsAmountChanged.AddDynamic(this, &AShooterHUD::UpdateSkillsCounter);
            UpgradesProgressStateComponent->OnUpgradesUnlocked.AddDynamic(this, &AShooterHUD::UpdateUnlockedProgress);
        }
    }
}

void AShooterHUD::SetAimVisibility(bool bState)
{
    bool bNullWidget = !IsValid(AimWidget);
    if (bNullWidget && !bState) return;
    if (bNullWidget) CreateAimWidget();
    if (IsValid(AimWidget)) AimWidget->SetAimVisibility(bState);
}

void AShooterHUD::SetAimTargetDetected(bool bState)
{
    if (!IsValid(AimWidget)) CreateAimWidget();
    if (IsValid(AimWidget)) AimWidget->SetAimTargetDetected(bState);
}

bool AShooterHUD::CreateAimWidget()
{
    if (IsValid(AimWidget) || !IsValid(AimWidgetClass)) return false;
    AimWidget = CreateWidget<UAimWidget>(GetWorld(), AimWidgetClass);
    if (!IsValid(AimWidget)) return false;
    AimWidget->AddToViewport();
    return true;
}

bool AShooterHUD::CreateShooterStatsWidget()
{
    if (IsValid(ShooterStatsWidget) || !IsValid(ShooterStatsWidgetClass)) return false;
    ShooterStatsWidget = CreateWidget<UShooterStatsWidget>(GetWorld(), ShooterStatsWidgetClass);
    if (!IsValid(ShooterStatsWidget)) return false;
    ShooterStatsWidget->AddToViewport();
    return true;
}

void AShooterHUD::SelectWeaponType(EWeaponType WeaponType)
{
    if (!IsValid(ShooterStatsWidget)) return;
    ShooterStatsWidget->SetActiveWeaponAs(WeaponType);
}

void AShooterHUD::SetVisibleSlots(const TSet<EWeaponType>& Slots)
{
    if (!IsValid(ShooterStatsWidget)) return;
    for (const auto& Slot : ShooterStatsWidget->GetWeaponSlots())
        ShooterStatsWidget->SetWeaponItemVisibility(Slot, Slots.Contains(Slot));

    if (!Slots.Contains(EWeaponType::Unarmed))
        ShooterStatsWidget->SetWeaponItemVisibility(EWeaponType::Unarmed, true);
}

void AShooterHUD::SetHealthValue(int32 Value, float Percent)
{
    if (IsValid(ShooterStatsWidget)) ShooterStatsWidget->SetHealthValue(Value, Percent);
}

void AShooterHUD::SetAmmoTo(EWeaponType WeaponType, int32 WeaponAmmo, int32 StockAmmo)
{
    if (IsValid(ShooterStatsWidget)) ShooterStatsWidget->SetAmmoTo(WeaponType, WeaponAmmo, StockAmmo);
}

void AShooterHUD::SetCurrentSkillsCountState(int32 Skills)
{
    if (IsValid(ShooterStatsWidget)) ShooterStatsWidget->SetSkillsCount(Skills);
	if (IsSkillsProgressionMenuOpened()) SkillsProgressionWidget->SetSkillsAvailableCount(Skills);
}

void AShooterHUD::SetCurrentUpgradesProgressionState(int32 Current, int32 Totla)
{
	if (IsSkillsProgressionMenuOpened()) SkillsProgressionWidget->SetUpgradesProgressState(Current, Totla);
}

bool AShooterHUD::IsSkillsProgressionMenuOpened() const
{
    return IsValid(SkillsProgressionWidget);
}

bool AShooterHUD::OpenSkillsProgressionMenu()
{
    if (IsSkillsProgressionMenuOpened() || !IsValid(SkillsProgressionWidgetClass)) return false;
    SkillsProgressionWidget = CreateWidget<USkillsProgressionWidget>(GetWorld(), SkillsProgressionWidgetClass);
    if (!IsValid(SkillsProgressionWidget)) return false;
    UButton* CloseMenuButton = SkillsProgressionWidget->GetCloseMenuButton();
    if (IsValid(CloseMenuButton))
        CloseMenuButton->OnClicked.AddDynamic(this, &AShooterHUD::CloseSkillsMenuAction);

    SkillsProgressionWidget->AddToViewport();
    SetSkillsMenuMusicPlayState(true);

    return true;
}

bool AShooterHUD::CloseSkillsProgressionMenu()
{
    if (!IsSkillsProgressionMenuOpened()) return false;
    UButton* CloseMenuButton = SkillsProgressionWidget->GetCloseMenuButton();
    if (IsValid(CloseMenuButton))
        CloseMenuButton->OnClicked.RemoveDynamic(this, &AShooterHUD::CloseSkillsMenuAction);

    SkillsProgressionWidget->RemoveFromParent();
    SkillsProgressionWidget = nullptr;
    SetSkillsMenuMusicPlayState(false);

    return true;
}

bool AShooterHUD::IsPauseMenuOpened() const
{
    return IsValid(PauseMenuWidget);
}

bool AShooterHUD::OpenPauseMenu()
{
    if (IsPauseMenuOpened() || !IsValid(PauseMenuWidgetClass)) return false;
    PauseMenuWidget = CreateWidget<UPauseMenuWidget>(GetWorld(), PauseMenuWidgetClass);
    if (!IsValid(PauseMenuWidget)) return false;
    UButton* ContinueButton = PauseMenuWidget->GetContinuePlayButton();
    if (IsValid(ContinueButton))
        ContinueButton->OnClicked.AddDynamic(this, &AShooterHUD::ClosePauseMenuAction);

    PauseMenuWidget->AddToViewport();
    SetPauseMenuMusicPlayState(true);
    return true;
}

bool AShooterHUD::ClosePauseMenu()
{
    if (!IsPauseMenuOpened()) return false;
    UButton* ContinueButton = PauseMenuWidget->GetContinuePlayButton();
    if (IsValid(ContinueButton))
        ContinueButton->OnClicked.RemoveDynamic(this, &AShooterHUD::ClosePauseMenuAction);

    PauseMenuWidget->RemoveFromParent();
    PauseMenuWidget = nullptr;
    SetPauseMenuMusicPlayState(false);

    return true;
}

void AShooterHUD::CloseSkillsMenuAction()
{
    CloseSkillsProgressionMenu();
    auto Controller = Cast<AShooterPlayerController>(GetOwningPlayerController());
    if (!IsValid(Controller)) return;
    Controller->SetInteractionMode(EPlayerInteractionMode::GameplayInputAndNotPause);
}

void AShooterHUD::ClosePauseMenuAction()
{
    ClosePauseMenu();
    auto Controller = Cast<AShooterPlayerController>(GetOwningPlayerController());
    if (!IsValid(Controller)) return;
    Controller->SetInteractionMode(EPlayerInteractionMode::GameplayInputAndNotPause);
}

void AShooterHUD::UpdateSkillsCounter(ESkillBalanceAction SkillBalanceAction, int32 Current, int32 Updated, int32 Total, int32 Previous)
{
	SetCurrentSkillsCountState(Current);
}

void AShooterHUD::UpdateUnlockedProgress(int32 Current, int32 Updated, int32 Total, int32 Previous)
{
    SetCurrentUpgradesProgressionState(Current, Total);
}

void AShooterHUD::SetSkillsMenuMusicPlayState(bool bState)
{
    auto GameMode = GetWorld()->GetAuthGameMode<AReignForceGameMode>();
    if (!IsValid(GameMode)) return;
    UBackgroundMusicComponent* MusicComponent = GameMode->GetBackgroundMusicComponent();
    if (!IsValid(MusicComponent)) return;

    if (bState)
    {
        MusicComponent->SetPauseForActiveGameplaySound(true);
        MusicComponent->ResetActiveSkillsMenuSound();
    }
    else
    {
        MusicComponent->StopActiveSkillsMenuSound();
        MusicComponent->SetPauseForActiveGameplaySound(false);
    }
}

void AShooterHUD::SetPauseMenuMusicPlayState(bool bState)
{
    auto GameMode = GetWorld()->GetAuthGameMode<AReignForceGameMode>();
    if (!IsValid(GameMode)) return;
    UBackgroundMusicComponent* MusicComponent = GameMode->GetBackgroundMusicComponent();
    if (!IsValid(MusicComponent)) return;
    MusicComponent->SetPauseForActiveGameplaySound(bState);
}
