// Fill out your copyright notice in the Description page of Project Settings.


#include "Spawners/Components/EnemySpawnerComponent.h"
#include "AIController.h"
#include "EngineUtils.h"
#include "Engine/TargetPoint.h"

#include "Spawners/EnemySpawnTargetPoint.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "Spawners/Interfaces/ShooterCharacterSpawner.h"
#include "Weapons/Components/WeaponSlotsSystem.h"
#include "Enemy/EnemyCharacter.h"


// Sets default values for this component's properties
UEnemySpawnerComponent::UEnemySpawnerComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SpawnBunch = 10;
}

TArray<AShooterCharacter*> UEnemySpawnerComponent::GetSpawnedShooters_Implementation()
{
	TArray<AShooterCharacter*> Spawned;
	for (auto& Enemy : SpawnedEnemies) Spawned.Add(Enemy.Get());
	return MoveTemp(Spawned);
}

int32 UEnemySpawnerComponent::GetAliveSpawnedCount_Implementation()
{
	return SpawnedEnemies.FilterByPredicate([] (const TObjectPtr<AEnemyCharacter>& Enemy)
	{
		return IsValid(Enemy) && !Enemy->IsDead();
	}).Num();
}

AShooterCharacter* UEnemySpawnerComponent::FindSpawnedByName_Implementation(const FString& ShooterName)
{
	TObjectPtr<AEnemyCharacter> const* EnemyPtr = SpawnedEnemies.FindByPredicate([&ShooterName] (const TObjectPtr<AEnemyCharacter>& Enemy)
	{
		return Enemy->GetName() == ShooterName;
	});
	return EnemyPtr ? *EnemyPtr : nullptr;
}

void UEnemySpawnerComponent::CollectSpawnPoints_Implementation()
{
	if (!GetWorld()) return;
	SpawnPoints.Empty();
    for (TActorIterator<AEnemySpawnTargetPoint> It(GetWorld()); It; ++It)
    {
        AEnemySpawnTargetPoint* FoundTarget = *It;
		SpawnPoints.Add(FoundTarget);
    }
}

bool UEnemySpawnerComponent::SpawnShooter_Implementation(const FOnShooterCreatedDeferred& OnShooterCreatedDeferred, ATargetPoint* SpawnPoint, TSubclassOf<AShooterCharacter> ToSpawn, int32 Amount)
{
	if (Amount < 1) return false;

	if (IsValid(SpawnPoint) && !SpawnPoint->IsA(AEnemySpawnTargetPoint::StaticClass())) return false;
	if (IsValid(ToSpawn) && !ToSpawn->IsChildOf(AEnemyCharacter::StaticClass())) return false;

	if (!(IsValid(SpawnPoint) || GetRandomSpawnPoint(SpawnPoint))) return false;
	if (!(IsValid(ToSpawn) || GetRandomEnemyToSpawn(ToSpawn))) return false;

	auto EnemySpawnTargetPoint = Cast<AEnemySpawnTargetPoint>(SpawnPoint);
	if (!(IsValid(EnemySpawnTargetPoint) && IsValid(ToSpawn))) return false;

	auto Enemy = GetWorld()->SpawnActorDeferred<AEnemyCharacter>(ToSpawn, FTransform::Identity);

	constexpr bool bReachableOnly = true;
	const FVector SpawnLocation = EnemySpawnTargetPoint->GetRandomCirclePoint(bReachableOnly);

	Enemy->SetActorLocation(SpawnLocation);
	OnShooterCreatedDeferred.ExecuteIfBound(Enemy);

	Enemy->FinishSpawning(FTransform::Identity);
	bool bSpawned = IsValid(Enemy);
	if (bSpawned) SpawnedEnemies.Add(Enemy);

	return bSpawned;
}

void UEnemySpawnerComponent::SpawnShooterBunch_Implementation(const FOnShooterCreatedDeferred& OnShooterCreatedDeferred, int32 Bunch)
{
	if (Bunch < 1) Bunch = SpawnBunch;
	for (int i = 0; i < Bunch; ++i)
		Execute_SpawnShooter(this, OnShooterCreatedDeferred, nullptr, nullptr, 1);
}

void UEnemySpawnerComponent::SpawnShooterBunchAsync_Implementation(const FOnShooterCreatedDeferred& OnShooterCreatedDeferred, const FOnShootersSpawned& OnShootersSpawned, int32 Bunch)
{
	if (Bunch < 1) Bunch = SpawnBunch;
	SpawnPerTick(OnShooterCreatedDeferred, OnShootersSpawned, Bunch);
}

void UEnemySpawnerComponent::ClearSpawnedShooters_Implementation(bool bDeadOnly)
{
	if (SpawnedEnemies.IsEmpty()) return;

	for (auto& Enemy : SpawnedEnemies)
	{
		if (!IsValid(Enemy)) continue;
		bool bShouldDestroy = !bDeadOnly || (bDeadOnly && Enemy->IsDead());
		UWeaponSlotsSystem* WeaponSlotsSystem = Enemy->GetWeaponSlotsSystem();
		if (IsValid(WeaponSlotsSystem)) WeaponSlotsSystem->RemoveAllWeapons();
		bool bDestroyed = Enemy->Destroy();
	}

	SpawnedEnemies.Empty();
}

void UEnemySpawnerComponent::SpawnPerTick(const FOnShooterCreatedDeferred& OnShooterCreatedDeferred, const FOnShootersSpawned& OnShootersSpawned, int32 Left)
{
	if (Left <= 0) return (void)OnShootersSpawned.ExecuteIfBound(true);
	Execute_SpawnShooter(this, OnShooterCreatedDeferred, nullptr, nullptr, 1);
	GetWorld()->GetTimerManager().SetTimerForNextTick([this, Left, OnShooterCreatedDeferred, OnShootersSpawned]
	{
		SpawnPerTick(OnShooterCreatedDeferred, OnShootersSpawned, Left - 1);
	});
}

bool UEnemySpawnerComponent::GetRandomEnemyToSpawn(TSubclassOf<AShooterCharacter>& Container) const
{
	if (EnemiesToSpawn.IsEmpty()) return false;
	Container = EnemiesToSpawn[FMath::RandRange(0, EnemiesToSpawn.Num() - 1)];
	return IsValid(Container);
}

bool UEnemySpawnerComponent::GetRandomSpawnPoint(ATargetPoint*& Container) const
{
	if (SpawnPoints.IsEmpty()) return false;
	TArray<AEnemySpawnTargetPoint*> ActivePoints = SpawnPoints.FilterByPredicate([] (const AEnemySpawnTargetPoint* Point)
	{
		return Point->bActiveToSpawn;
	});
	if (ActivePoints.IsEmpty()) return false;
	Container = ActivePoints[FMath::RandRange(0, ActivePoints.Num() - 1)];
	return IsValid(Container);
}
