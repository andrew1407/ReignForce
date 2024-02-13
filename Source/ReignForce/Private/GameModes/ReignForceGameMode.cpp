// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameModes/ReignForceGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"

#include "GameModes/ShooterEquipmentDataAsset.h"
#include "GameModes/Components/BackgroundMusicComponent.h"
#include "GameStates/ShooterGameState.h"

#include "ShooterCharacter/ShooterCharacter.h"
#include "PlayerCharacter/PlayerCharacter.h"
#include "Weapons/Types/Weapon.h"
#include "Weapons/Types/Weapon.h"


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
		ShooterGameState->OnRoundStarted.AddDynamic(this, &AReignForceGameMode::PlayBackgroundGameplayMusic);
		ShooterGameState->OnRoundStarted.AddDynamic(this, &AReignForceGameMode::RefillPlayerAmmoOnRoundStart);

		ShooterGameState->OnRoundEnded.AddDynamic(this, &AReignForceGameMode::StopBackgroundGameplayMusic);
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

void AReignForceGameMode::PlayBackgroundGameplayMusic(bool bStartedByPlayer)
{
	if (!IsValid(BackgroundMusicComponent)) return;

	BackgroundMusicComponent->ResetActiveGameplaySound();
}

void AReignForceGameMode::RefillPlayerAmmoOnRoundStart(bool bStartedByPlayer)
{
	RefillPlayerAmmo();
}

void AReignForceGameMode::StopBackgroundGameplayMusic(bool bPlayerWin)
{
	if (!IsValid(BackgroundMusicComponent)) return;

	BackgroundMusicComponent->StopActiveGameplaySound();
}

void AReignForceGameMode::EndRoundOnPlayerDeath(AShooterCharacter* Character, AActor* Cause)
{
	auto ShooterGameState = GetGameState<AShooterGameState>();
	if (!IsValid(ShooterGameState)) return;

	ShooterGameState->StopEnemiesAttackForTarget(Character);
	constexpr bool bPlayerWin = false;
	ShooterGameState->EndRound(bPlayerWin);
}
