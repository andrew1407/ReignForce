// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameModes/ReignForceGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"

#include "GameModes/ShooterEquipmentDataAsset.h"
#include "GameModes/Components/BackgroundMusicComponent.h"
#include "GameStates/ShooterGameState.h"

#include "UI/ShooterHUD.h"
#include "UI/Rounds/RestartLevelAfterLoseWidget.h"

#include "ShooterCharacter/ShooterCharacter.h"
#include "Weapons/Types/Weapon.h"
#include "Weapons/Types/Weapon.h"

#include "PlayerCharacter/PlayerCharacter.h"
#include "ShooterCharacter/PointsModifierType.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"

AReignForceGameMode::AReignForceGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// set default pawn class to our Blueprinted character
	// static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	// if (PlayerPawnBPClass.Class) DefaultPawnClass = PlayerPawnBPClass.Class;

	BackgroundMusicComponent = CreateDefaultSubobject<UBackgroundMusicComponent>(GET_MEMBER_NAME_CHECKED(AReignForceGameMode, BackgroundMusicComponent));
}

void AReignForceGameMode::BeginPlay()
{
	Super::BeginPlay();

	auto ShooterGameState = GetGameState<AShooterGameState>();
	if (IsValid(ShooterGameState))
	{
		ShooterGameState->OnRoundStarted.AddDynamic(this, &AReignForceGameMode::HandleRoundStart);
		ShooterGameState->OnRoundEnded.AddDynamic(this, &AReignForceGameMode::HandleRoundEnd);
	}

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (IsValid(PlayerController))
	{
		auto PlayerCharacter = PlayerController->GetPawn<APlayerCharacter>();
		if (IsValid(PlayerCharacter))
			PlayerCharacter->OnPerish.AddDynamic(this, &AReignForceGameMode::EndRoundOnPlayerDeath);
	}
}

bool AReignForceGameMode::IsShooterEquipmentValid() const
{
	return IsValid(ShooterEquipment) && ShooterEquipment->IsValidLowLevel();
}

TSoftClassPtr<AWeapon> AReignForceGameMode::GetWeaponModel(EWeaponType WeaponType, int32 Index) const
{
	if (!IsShooterEquipmentValid()) return nullptr;
	FWeaponModelTypes* ModelTypes = ShooterEquipment->WeaponModels.Find(WeaponType);
	if (!ModelTypes) return nullptr;
	if (!ModelTypes->Models.IsValidIndex(Index)) return nullptr;
	return ModelTypes->Models[Index];
}

void AReignForceGameMode::RefillShooterAmmo(AShooterCharacter* Shooter)
{
	if (IsValid(Shooter)) Shooter->SetFullAmmoForAllAvailableFirearms();
}

void AReignForceGameMode::RefillPlayerAmmo()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!IsValid(PlayerController)) return;
	auto Player = PlayerController->GetPawn<AShooterCharacter>();
	if (!IsValid(Player)) return;
	RefillShooterAmmo(Player);
}

void AReignForceGameMode::AtivateRoundReloadCoundown(bool bResetIfStarted)
{
	bool bCountdownActivated = GetWorldTimerManager().IsTimerActive(ReloadRoundTimer);
	if (bCountdownActivated)
	{
		if (bResetIfStarted) GetWorldTimerManager().ClearTimer(ReloadRoundTimer);
		else return;
	}

	constexpr float InRate = 1;
	constexpr bool bInLoop = true;
	constexpr float InFirstDelay = 0;
	GetWorldTimerManager().SetTimer(ReloadRoundTimer, this, &AReignForceGameMode::OnReloadLevelTimerHandle, InRate, bInLoop, InFirstDelay);
}

void AReignForceGameMode::HandleRoundStart(bool bStartedByPlayer)
{
	if (IsValid(BackgroundMusicComponent))
	{
		BackgroundMusicComponent->ResetActiveGameplaySound();
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		bool bShouldPause = IsValid(PlayerController) && PlayerController->IsPaused();
		if (bShouldPause) BackgroundMusicComponent->SetPauseForActiveGameplaySound(true);
	}

	RefillPlayerAmmo();
	SetPlayerHealthToFull();

	AShooterHUD* ShooterHUD = GetShooterHUD();
	if (IsValid(ShooterHUD)) ShooterHUD->CloseStartRoundWidget();
}

void AReignForceGameMode::HandleRoundEnd(bool bPlayerWin)
{
	if (IsValid(BackgroundMusicComponent)) BackgroundMusicComponent->StopActiveGameplaySound();

	if (bPlayerWin)
	{
		AShooterHUD* ShooterHUD = GetShooterHUD();
		if (IsValid(ShooterHUD)) ShooterHUD->OpenStartRoundWidget();
	}
}

void AReignForceGameMode::EndRoundOnPlayerDeath(AShooterCharacter* Character, AActor* Cause)
{
	auto ShooterGameState = GetGameState<AShooterGameState>();
	if (IsValid(ShooterGameState))
	{
		ShooterGameState->StopEnemiesAttackForTarget(Character);
		constexpr bool bPlayerWin = false;
		ShooterGameState->EndRound(bPlayerWin);
	}

	AtivateRoundReloadCoundown();
}

void AReignForceGameMode::SetPlayerHealthToFull()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!IsValid(PlayerController)) return;
	auto Player = PlayerController->GetPawn<AShooterCharacter>();
	if (!IsValid(Player)) return;
	UShooterStatsComponent* Stats = Player->GetStatsComponent();
	if (!IsValid(Stats)) return;

    const float HealthBeforeUpgrade = Player->GetHealthPoints();
	Stats->RefillHealthToMax();
    const float HealthAfterUpgrade = Player->GetHealthPoints();
    const float Diff = HealthAfterUpgrade - HealthBeforeUpgrade;
	Player->OnHealthChanged.Broadcast(Diff, HealthAfterUpgrade, HealthBeforeUpgrade, EPointsModifierType::SetManually);
}

void AReignForceGameMode::OnReloadLevelTimerHandle()
{

	AShooterHUD* ShooterHUD = GetShooterHUD();
	if (!IsValid(ShooterHUD)) return;
	bool bRestartLevelAfterLoseWidgetOpened = ShooterHUD->IsRestartLevelAfterLoseWidgetOpened();
	if (!bRestartLevelAfterLoseWidgetOpened) ShooterHUD->OpenRestartLevelAfterLoseWidget();

	URestartLevelAfterLoseWidget* RestartLevelWidget = ShooterHUD->GetRestartLevelAfterLoseWidget();
	if (!IsValid(RestartLevelWidget)) return;

	if (RestartLevelWidget->IsCountdownFull()) RestartLevelWidget->ResetCountdownMessage();
	if (!bRestartLevelAfterLoseWidgetOpened) return;
	RestartLevelWidget->DecreaseCurrentSecondsCountdown();

	if (RestartLevelWidget->IsNoSecondsLeft())
	{
        if (GetWorldTimerManager().IsTimerActive(ReloadRoundTimer))
			GetWorldTimerManager().ClearTimer(ReloadRoundTimer);
		auto ShooterGameState = GetGameState<AShooterGameState>();
		if (IsValid(ShooterGameState)) ShooterGameState->RestartLevel();
	}
}

AShooterHUD* AReignForceGameMode::GetShooterHUD() const
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	return IsValid(PlayerController) ? PlayerController->GetHUD<AShooterHUD>() : nullptr;
}
