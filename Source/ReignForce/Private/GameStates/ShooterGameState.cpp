// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/ShooterGameState.h"
#include "Kismet/GameplayStatics.h"

#include "ShooterSaveGame.h"
#include "GameStates/Components/UpgradesProgressStateComponent.h"
#include "GameStates/Components/RoundDifficultyProgression.h"

#include "PlayerCharacter/PlayerCharacter.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"
#include "Weapons/Components/WeaponSlotsSystem.h"

#include "Enemy/EnemyCharacter.h"
#include "Enemy/ShooterAIController.h"
#include "Spawners/Components/EnemySpawnerComponent.h"
#include "Spawners/Components/EnemyEquipSystem.h"


namespace
{
	constexpr uint8 SLOT_STATE_INDEX = 0;
    bool LOG_ENEMY_SPAWN_ACTIONS = false;
}

AShooterGameState::AShooterGameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CharacterStateSlot = TEXT("ShooterSaveSlot");

	EnemySpawnerComponent = CreateDefaultSubobject<UEnemySpawnerComponent>(GET_MEMBER_NAME_CHECKED(AShooterGameState, EnemySpawnerComponent));
	EnemyEquipSystem = CreateDefaultSubobject<UEnemyEquipSystem>(GET_MEMBER_NAME_CHECKED(AShooterGameState, EnemyEquipSystem));
	UpgradesProgressStateComponent = CreateDefaultSubobject<UUpgradesProgressStateComponent>(GET_MEMBER_NAME_CHECKED(AShooterGameState, UpgradesProgressStateComponent));
	RoundDifficultyProgression = CreateDefaultSubobject<URoundDifficultyProgression>(GET_MEMBER_NAME_CHECKED(AShooterGameState, RoundDifficultyProgression));

	RoundState = ERoundState::None;
}

void AShooterGameState::BeginPlay()
{
	Super::BeginPlay();

	OnRoundEnded.AddDynamic(this, &AShooterGameState::AddSkillsForRoundWin);
}

bool AShooterGameState::SavePlayerCharacterState(APlayerCharacter* Player)
{
	bool bPlayerValid = IsValid(Player) && IsValid(Player->GetSkillsSystem()) && IsValid(Player->GetStatsComponent());
	if (!bPlayerValid) return false;

	auto Save = Cast<UShooterSaveGame>(UGameplayStatics::CreateSaveGameObject(UShooterSaveGame::StaticClass()));
	if (!IsValid(Save)) return false;

	Save->RanksProgression = Player->GetSkillsSystem()->RanksProgression;
	Save->AvailableSlots = Player->GetSkillsSystem()->AvailableSlots;
	Save->WeaponModels = Player->GetStatsComponent()->WeaponModels;
	Save->PlayerProgressionBalance = GetCurrentProgress();

	return UGameplayStatics::SaveGameToSlot(Save, CharacterStateSlot, SLOT_STATE_INDEX);
}

bool AShooterGameState::LoadPlayerCharacterState(APlayerCharacter* Player)
{
	bool bPlayerValid = IsValid(Player) && IsValid(Player->GetSkillsSystem()) && IsValid(Player->GetStatsComponent());
	if (!bPlayerValid) return false;

	auto Save = Cast<UShooterSaveGame>(UGameplayStatics::LoadGameFromSlot(CharacterStateSlot, SLOT_STATE_INDEX));
	if (!IsValid(Save)) return false;

	Player->GetSkillsSystem()->RanksProgression = Save->RanksProgression;
	Player->GetSkillsSystem()->AvailableSlots = Save->AvailableSlots;
	Player->GetStatsComponent()->WeaponModels = Save->WeaponModels;

	if (IsValid(UpgradesProgressStateComponent))
	{
		const FPlayerProgressionBalance Balance = Save->PlayerProgressionBalance;
		UpgradesProgressStateComponent->SetUpgradesCount(Balance.UpgradesUnlocked);
		UpgradesProgressStateComponent->SetSkillsCount(Balance.SkillsAvailable);
		UpgradesProgressStateComponent->SetSkillsSpentCount(Balance.SkillsSpent);
	}

	return true;
}

bool AShooterGameState::ClearPlayerCharacterState()
{
	bool bDeleted = UGameplayStatics::DeleteGameInSlot(CharacterStateSlot, SLOT_STATE_INDEX);
	return bDeleted;
}

FPlayerProgressionBalance AShooterGameState::GetCurrentProgress() const
{
	if (!IsValid(UpgradesProgressStateComponent)) return {};
	return UpgradesProgressStateComponent->GetProgressionState();
}

AEnemyCharacter* AShooterGameState::FindSpawnedEnemyByName(const FString& EnemyName) const
{
	if (IsValid(EnemySpawnerComponent)) return nullptr;
	return Cast<AEnemyCharacter>(EnemySpawnerComponent->Execute_FindSpawnedByName(EnemySpawnerComponent, EnemyName));
}

bool AShooterGameState::ActivateEnemiesSpawnByDefaultAmount()
{
	if (RoundState != ERoundState::None) return false;
	RoundState = ERoundState::Starting;

	constexpr int8 DefaultFlag = -1;
	return SpawnEnemies(DefaultFlag);
}

bool AShooterGameState::ActivateEnemiesSpawnByProgression()
{
	if (RoundState != ERoundState::None) return false;
	RoundState = ERoundState::Starting;

	bool bCanHandle = IsValid(RoundDifficultyProgression) && IsValid(UpgradesProgressStateComponent);
	if (!bCanHandle) return false;

	FPlayerProgressionBalance Balance = UpgradesProgressStateComponent->GetProgressionState();
	int32 ToSpawn = RoundDifficultyProgression->GetEnemiesCountBySkills(Balance.GetSkillsGained());
	if (ToSpawn <= 0) return false;

	return SpawnEnemies(ToSpawn);
}

void AShooterGameState::ClearLevelFromSpawnedEnemies(bool bDeadOnly)
{
	if (!IsValid(EnemySpawnerComponent)) return;
	
	TArray<AShooterCharacter*> Enemies = EnemySpawnerComponent->Execute_GetSpawnedShooters(EnemySpawnerComponent);
	for (auto Enemy : Enemies)
	{
		if (!IsValid(Enemy)) continue;
		bool bShouldDestroy = !bDeadOnly || (bDeadOnly && Enemy->IsDead());
		if (!bShouldDestroy) continue;
		UWeaponSlotsSystem* WeaponSlotsSystem = Enemy->GetWeaponSlotsSystem();
		if (IsValid(WeaponSlotsSystem)) WeaponSlotsSystem->RemoveAllWeapons();
		Enemy->Destroy();
	}
}

void AShooterGameState::StopEnemiesAttackForTarget(AActor* Target, float DelaySec)
{
	if (!(Target && IsValid(EnemySpawnerComponent))) return;

	TArray<AShooterCharacter*> Enemies = EnemySpawnerComponent->Execute_GetSpawnedShooters(EnemySpawnerComponent);
	for (auto Enemy : Enemies)
	{
		if (!IsValid(Enemy)) continue;
		auto ShooterAIController = Enemy->GetController<AShooterAIController>();
		if (!IsValid(ShooterAIController)) continue;
		AActor* ShooterTarget = ShooterAIController->GetTargetFocus();
		if (Target != ShooterTarget) continue;

		GetWorldTimerManager().SetTimerForNextTick([this, DelaySec, ShooterAIController]
        {
			if (!IsValid(ShooterAIController)) return;
            FTimerHandle TimerHandle;
			auto ClearFocus = [this, ShooterAIController] { ShooterAIController->ResetTargetFocus(nullptr); };
			GetWorldTimerManager().SetTimer(TimerHandle, ClearFocus, DelaySec, false);
        });
	}
}

bool AShooterGameState::StartRound()
{
	if (RoundState != ERoundState::None) return false;

	constexpr bool bDeadOnly = false;
	ClearLevelFromSpawnedEnemies(bDeadOnly);
	return ActivateEnemiesSpawnByProgression();
}

void AShooterGameState::RestartLevel()
{
	UWorld* Level = GetWorld();
	constexpr bool bAbsolute = false;
	UGameplayStatics::OpenLevel(Level, *GetNameSafe(Level), bAbsolute);
}

bool AShooterGameState::EndRound(bool bWithSuccess)
{
	if (RoundState != ERoundState::Going) return false;

	RoundState = ERoundState::None;
	OnRoundEnded.Broadcast(bWithSuccess);

	if (LOG_ENEMY_SPAWN_ACTIONS)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, TEXT("Round is ended"));
	}

	return true;
}

void AShooterGameState::EquipEnemyShooter(AShooterCharacter* Shooter)
{
	if (!IsValid(Shooter)) return;
	Shooter->OnPerish.AddDynamic(this, &AShooterGameState::OnEnemyPerished);

	if (!IsValid(EnemyEquipSystem)) return;
	EnemyEquipSystem->Execute_ProvideWithAvailableWeapons(EnemyEquipSystem, Shooter);
	EnemyEquipSystem->Execute_ProvideWithRanksProgression(EnemyEquipSystem, Shooter);
	// EnemyEquipSystem->Execute_AdjustBehavior(EnemyEquipSystem, Shooter);

	auto Enemy = Cast<AEnemyCharacter>(Shooter);
	if (!IsValid(Enemy)) return;

	if (LOG_ENEMY_SPAWN_ACTIONS)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Magenta, FString::Printf(TEXT("EquipEnemyShooter(AEnemyCharacter* Enemy): %s"), *Enemy->GetName()));
	}
}

void AShooterGameState::OnEnemiesSpawned(bool bSuccess)
{
	if (!bSuccess) return;
	if (!(IsValid(EnemyEquipSystem) && IsValid(EnemySpawnerComponent))) return;

	TArray<AShooterCharacter*> Spawned = EnemySpawnerComponent->Execute_GetSpawnedShooters(EnemySpawnerComponent);
	EnemyEquipSystem->Execute_DistributeBehaviors(EnemyEquipSystem, Spawned);

	RoundState = ERoundState::Going;
	constexpr bool bAbsolute = true;
	OnRoundStarted.Broadcast(bAbsolute);

	if (LOG_ENEMY_SPAWN_ACTIONS)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Magenta, FString::Printf(TEXT("EnemiesSpawned(bool bSuccess): %d"), bSuccess));
	}
}

void AShooterGameState::AddSkillsForRoundWin(bool bPlayerWin)
{
	if (!bPlayerWin) return;
	bool bCanHandle = IsValid(RoundDifficultyProgression) && IsValid(UpgradesProgressStateComponent);
	if (!bCanHandle) return;

	if (UpgradesProgressStateComponent->IsFullOfSkills()) return;
	const int32 SkillsReward = RoundDifficultyProgression->GetSkillsRewardForRoundWin();
	bool bAccured = UpgradesProgressStateComponent->AddSkills(SkillsReward);
}

void AShooterGameState::OnEnemyPerished(AShooterCharacter* Enemy, AActor* Cause)
{
	if (!(IsValid(Enemy) && IsValid(EnemySpawnerComponent))) return;

	if (LOG_ENEMY_SPAWN_ACTIONS)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, FString::Printf(TEXT("%s is dead"), *GetNameSafe(Enemy)));
	}

	const int32 Alive = EnemySpawnerComponent->Execute_GetAliveSpawnedCount(EnemySpawnerComponent);
	bool bAllEnemiesDefeated = Alive <= 0;
	if (!bAllEnemiesDefeated) return;

	constexpr bool bPlayerWin = true;
	EndRound(bPlayerWin);
}

bool AShooterGameState::SpawnEnemies(int32 ToSpawn)
{
	if (!IsValid(EnemySpawnerComponent)) return false;

	EnemySpawnerComponent->Execute_CollectSpawnPoints(EnemySpawnerComponent);

	FOnShooterCreatedDeferred OnShooterCreatedDeferred;
	OnShooterCreatedDeferred.BindDynamic(this, &AShooterGameState::EquipEnemyShooter);

	FOnShootersSpawned OnShootersSpawned;
	OnShootersSpawned.BindDynamic(this, &AShooterGameState::OnEnemiesSpawned);

	EnemySpawnerComponent->Execute_SpawnShooterBunchAsync(EnemySpawnerComponent, OnShooterCreatedDeferred, OnShootersSpawned, ToSpawn);
	return true;
}
