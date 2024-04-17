// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ShooterHUD.h"
#include "Kismet/GameplayStatics.h"

#include "Components/Button.h"
#include "Components/AudioComponent.h"

#include "GameModes/ReignForceGameMode.h"
#include "GameModes/Components/BackgroundMusicComponent.h"

#include "GameStates/ShooterGameState.h"
#include "GameStates/Components/UpgradesProgressStateComponent.h"

#include "PlayerCharacter/ShooterPlayerController.h"
#include "PlayerCharacter/PlayerInteractionMode.h"

#include "UI/Components/UIMessageLoggerComponent.h"
#include "UI/AimWidget.h"
#include "UI/ShooterStatsWidget.h"
#include "UI/Skills/SkillsProgressionWidget.h"
#include "UI/PauseMenu/PauseMenuWidget.h"

#include "UI/Rounds/RestartLevelAfterLoseWidget.h"
#include "UI/Rounds/RoundProgressWidget.h"
#include "UI/Rounds/StartRoundWidget.h"


AShooterHUD::AShooterHUD(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MessageLoggerComponent = CreateDefaultSubobject<UUIMessageLoggerComponent>(GET_MEMBER_NAME_CHECKED(AShooterHUD, MessageLoggerComponent));

    AlternativeSkillsProgressionSoundChance = .3f;
}

void AShooterHUD::BeginPlay()
{
    Super::BeginPlay();

    if (IsValid(MessageLoggerComponent)) MessageLoggerComponent->OpenGameMessageLogger();

	auto GameState = GetWorld()->GetGameState<AShooterGameState>();
    if (IsValid(GameState))
    {
        UUpgradesProgressStateComponent* PlayerProgressComponent = GetPlayerProgressComponent();
        if (IsValid(PlayerProgressComponent))
        {
            PlayerProgressComponent->OnSkillsAmountChanged.AddDynamic(this, &AShooterHUD::UpdateSkillsCounter);
            PlayerProgressComponent->OnUpgradesUnlocked.AddDynamic(this, &AShooterHUD::UpdateUnlockedProgress);
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

    USoundBase* SkillsProgressionSound = GetSkillsProgressionSound();
    if (IsValid(SkillsProgressionSound))
    {
        constexpr float PitchMultiplier = 1;
        constexpr float SoundVolume = 1;
	    UGameplayStatics::SpawnSound2D(GetWorld(), SkillsProgressionSound, SoundVolume, PitchMultiplier);
    }

    if (IsValid(MessageLoggerComponent)) MessageLoggerComponent->ClearLogger();

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

    if (IsValid(PauseMenuWidgetSound))
    {
        constexpr float PitchMultiplier = 1;
        constexpr float SoundVolume = 1;
	    UGameplayStatics::SpawnSound2D(GetWorld(), PauseMenuWidgetSound, SoundVolume, PitchMultiplier);
    }

    if (IsValid(MessageLoggerComponent)) MessageLoggerComponent->ClearLogger();

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

bool AShooterHUD::IsRestartLevelAfterLoseWidgetOpened() const
{
    return IsValid(RestartLevelAfterLoseWidget);
}

bool AShooterHUD::OpenRestartLevelAfterLoseWidget()
{
    if (IsRestartLevelAfterLoseWidgetOpened() || !IsValid(RestartLevelAfterLoseClass)) return false;
    RestartLevelAfterLoseWidget = CreateWidget<URestartLevelAfterLoseWidget>(GetWorld(), RestartLevelAfterLoseClass);
    if (!IsValid(RestartLevelAfterLoseWidget)) return false;
    RestartLevelAfterLoseWidget->AddToViewport();
    return true;
}

bool AShooterHUD::CloseRestartLevelAfterLoseWidget()
{
    if (!IsRestartLevelAfterLoseWidgetOpened()) return false;
    RestartLevelAfterLoseWidget->RemoveFromParent();
    RestartLevelAfterLoseWidget = nullptr;
    return true;
}

bool AShooterHUD::IsRoundProgressWidgetOpened() const
{
    return IsValid(RoundProgressWidget);
}

bool AShooterHUD::OpenRoundProgressWidget()
{
    if (IsRoundProgressWidgetOpened() || !IsValid(RoundProgressClass)) return false;
    RoundProgressWidget = CreateWidget<URoundProgressWidget>(GetWorld(), RoundProgressClass);
    if (!IsValid(RoundProgressWidget)) return false;
    RoundProgressWidget->AddToViewport();
    return true;
}

bool AShooterHUD::IsStartRoundWidgetOpened() const
{
    return IsValid(StartRoundWidget);
}

bool AShooterHUD::OpenStartRoundWidget()
{
    if (IsStartRoundWidgetOpened() || !IsValid(StartRoundClass)) return false;
    StartRoundWidget = CreateWidget<UStartRoundWidget>(GetWorld(), StartRoundClass);
    if (!IsValid(StartRoundWidget)) return false;
    StartRoundWidget->AddToViewport();
    return true;
}

bool AShooterHUD::CloseStartRoundWidget()
{
    if (!IsStartRoundWidgetOpened()) return false;
    StartRoundWidget->RemoveFromParent();
    StartRoundWidget = nullptr;
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
    if (bState) MusicComponent->ResetActivePauseMenuSound();
    else MusicComponent->StopActivePauseMenuSound();
}

UUpgradesProgressStateComponent* AShooterHUD::GetPlayerProgressComponent() const
{
    auto GameState = GetWorld()->GetGameState<AShooterGameState>();
    return IsValid(GameState) ? GameState->GetUpgradesProgressStateComponent() : nullptr;
}

USoundBase* AShooterHUD::GetSkillsProgressionSound() const
{
    UUpgradesProgressStateComponent* PlayerProgressComponent = GetPlayerProgressComponent();
    if (!IsValid(PlayerProgressComponent)) return nullptr;
    
    float AlternativeSoundChance = PlayerProgressComponent->GetUpgradesUnlocked() / PlayerProgressComponent->GetMaxUpgrades();
    AlternativeSoundChance *= AlternativeSkillsProgressionSoundChance;

    bool bSelectAlternative = AlternativeSoundChance >= FMath::FRand();
    return bSelectAlternative ? SkillsProgressionWidgetAlternativeSound : SkillsProgressionWidgetSound;
}
