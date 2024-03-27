// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

#include "Engine/DamageEvents.h"
#include "Engine/LocalPlayer.h"
#include "Engine/DamageEvents.h"

#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/TimelineComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "PlayerCharacter/ShooterPlayerController.h"
#include "PlayerCharacter/PlayerInteractionMode.h"

#include "ShooterCharacter/AimAvailability.h"
#include "ShooterCharacter/TeamID.h"
#include "ShooterCharacter/RanksProgressionSource.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"

#include "Weapons/Components/WeaponSlotsSystem.h"
#include "Weapons/Components/CombatSystem.h"
#include "Weapons/Types/Weapon.h"
#include "Weapons/Types/FirearmsWeapon.h"

#include "UI/ShooterHUD.h"
#include "UI/ShooterStatsWidget.h"
#include "UI/Skills/SkillsProgressionWidget.h"
#include "UI/Components/UIMessageLoggerComponent.h"

#include "GameStates/ShooterGameState.h"
#include "GameStates/Components/UpgradesProgressStateComponent.h"
#include "ReinforcementLearning/Interfaces/AICommandsStrategy.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    // Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(GET_MEMBER_NAME_CHECKED(APlayerCharacter, CameraBoom));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->SocketOffset = { 0, 50, 50 };
	CameraBoom->TargetOffset = { 0, 0, 100 };
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(GET_MEMBER_NAME_CHECKED(APlayerCharacter, FollowCamera));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	AimingTimeline = CreateDefaultSubobject<UTimelineComponent>(GET_MEMBER_NAME_CHECKED(APlayerCharacter, AimingTimeline));

	GetStatsComponent()->bHealthRegeneratorEnabled = true;
	TeamId = ETeamID::Player;
    WeaponSlotInputs = {
		{ FVector2D(0, 1), EWeaponType::Unarmed },
		{ FVector2D(1, 0), EWeaponType::Rifle },
		{ FVector2D(-0.f, -1), EWeaponType::Pistol },
		{ FVector2D(-1, -0.f), EWeaponType::Melee },
	};
}

void APlayerCharacter::BeginPlay()
{
	// Call the base class
	Super::BeginPlay();

	OnHealthChanged.AddDynamic(this, &APlayerCharacter::UpdateHealthBar);
	OnStatsDefined.AddDynamic(this, &APlayerCharacter::HandleStatsDefined);

    auto PlayerController = GetController<APlayerController>();
	if (IsValid(PlayerController))
	{
    	//Add Input Mapping Context
		auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (IsValid(Subsystem) && Inputs.MappingContext) Subsystem->AddMappingContext(Inputs.MappingContext, 0);
	
		ShooterHUD = PlayerController->GetHUD<AShooterHUD>();
		if (ShooterHUD.IsValid()) ShooterHUD->CreateShooterStatsWidget();
	}

	if (!IsShooterComponentsValid()) return;

	bool bShouldInitialize = RanksProgressionSource == ERanksProgressionSource::Default;
	if (RanksProgressionSource == ERanksProgressionSource::FromSave) bShouldInitialize = !LoadProgress();
	if (bShouldInitialize) GetSkillsSystem()->InitializeDefaultRanks();

	if (ShooterHUD.IsValid())
	{
		ShooterHUD->SetAimVisibility(GetSkillsSystem()->GetAimAvailabilityOrDefault() != EAimAvailability::NoAim);
		ShooterHUD->SetVisibleSlots(GetSkillsSystem()->AvailableSlots);

		auto GameState = GetWorld()->GetGameState<AShooterGameState>();
		if (IsValid(GameState))
		{
			UUpgradesProgressStateComponent* UpgradesProgressStateComponent = GameState->GetUpgradesProgressStateComponent();
			if (IsValid(UpgradesProgressStateComponent))
			{
				const int32 SkillsCount = UpgradesProgressStateComponent->GetSkillsCount();
				ShooterHUD->SetCurrentSkillsCountState(SkillsCount);
			}
		}

		ShooterHUD->OpenStartRoundWidget();
	}

    DefineStats();
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	HandleAimTrace();
	TryReenerateHealth(DeltaSeconds);
}

bool APlayerCharacter::GetAimingLook_Implementation(FVector& Direction, FRotator& Rotator) const
{
	if (!IsValid(FollowCamera)) return false;
	Direction = FollowCamera->GetForwardVector();
	Rotator = FollowCamera->GetComponentTransform().Rotator();
	return true;
}

float APlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageTaken = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (DamageTaken == 0) return DamageTaken;
	bool bPerished = IsValid(GetStatsComponent()) && GetStatsComponent()->HasNoHealth();
	if (bPerished) Perish(DamageCauser);
	return DamageTaken;
}

void APlayerCharacter::HandleStatsDefined(bool bSuccess)
{
	if (!bSuccess) return;

	DefineDamageHitReactions();
    UpdateActiveWeaponHUD();
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	auto EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!IsValid(EnhancedInputComponent))
	{
		UE_LOG(LogTemplateCharacter, Error,
			TEXT("'%s' %s %s"),
			*GetNameSafe(this),
			TEXT("Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system."),
			TEXT("If you intend to use the legacy system, then you will need to update this C++ file.")
		);
		return;
	}

	// Jumping
	if (Inputs.JumpAction)
	{
		EnhancedInputComponent->BindAction(Inputs.JumpAction, ETriggerEvent::Started, this, &AShooterCharacter::Jump);
		EnhancedInputComponent->BindAction(Inputs.JumpAction, ETriggerEvent::Completed, this, &AShooterCharacter::StopJumping);
	}

	// Moving
	if (Inputs.MoveAction)
		EnhancedInputComponent->BindAction(Inputs.MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::MoveAction);

	// Looking
	if (Inputs.LookAction)
		EnhancedInputComponent->BindAction(Inputs.LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::LookAction);

	// Aiming
	if (Inputs.AimAction)
	{
		EnhancedInputComponent->BindAction(Inputs.AimAction, ETriggerEvent::Started, this, &AShooterCharacter::StartAiming);
		EnhancedInputComponent->BindAction(Inputs.AimAction, ETriggerEvent::Completed, this, &AShooterCharacter::StopAiming);
	}

	if (Inputs.AttackAction)
	{
		EnhancedInputComponent->BindAction(Inputs.AttackAction, ETriggerEvent::Started, this, &AShooterCharacter::StartAttacking);
		EnhancedInputComponent->BindAction(Inputs.AttackAction, ETriggerEvent::Triggered, this, &AShooterCharacter::OnDuringAttack);
		EnhancedInputComponent->BindAction(Inputs.AttackAction, ETriggerEvent::Completed, this, &AShooterCharacter::StopAttacking);
	}

	if (Inputs.SwitchAimFocusAction)
		EnhancedInputComponent->BindAction(Inputs.SwitchAimFocusAction, ETriggerEvent::Started, this, &APlayerCharacter::SwitchAimFocusAction);

	if (Inputs.SwitchWeaponAction)
		EnhancedInputComponent->BindAction(Inputs.SwitchWeaponAction, ETriggerEvent::Started, this, &APlayerCharacter::SwitchWeaponAction);
	
	if (Inputs.ReloadWeaponAction)
		EnhancedInputComponent->BindAction(Inputs.ReloadWeaponAction, ETriggerEvent::Started, this, &APlayerCharacter::ReloadWeaponAction);

	if (Inputs.TogglePauseMenuAction)
		EnhancedInputComponent->BindAction(Inputs.TogglePauseMenuAction, ETriggerEvent::Started, this, &APlayerCharacter::TogglePauseMenuAction);

	if (Inputs.ToggleSkillsMenuAction)
		EnhancedInputComponent->BindAction(Inputs.ToggleSkillsMenuAction, ETriggerEvent::Started, this, &APlayerCharacter::ToggleSkillsMenuAction);
	
	if (Inputs.ResetRoundAction)
		EnhancedInputComponent->BindAction(Inputs.ResetRoundAction, ETriggerEvent::Started, this, &APlayerCharacter::ResetRoundAction);

	if (Inputs.QuickSaveAction)
		EnhancedInputComponent->BindAction(Inputs.QuickSaveAction, ETriggerEvent::Started, this, &APlayerCharacter::QuickSaveAction);
}

bool APlayerCharacter::SaveProgress()
{
	if (!ShooterHUD.IsValid()) return false;
	UUIMessageLoggerComponent* MessageLoggerComponent = ShooterHUD->GetMessageLoggerComponent();
	if (!IsValid(MessageLoggerComponent)) return false;

	if (IsDead())
	{
		MessageLoggerComponent->AddErrorLoggerMessageSavingOnPlayerDead();
		return false;
	}

	auto GameState = GetWorld()->GetGameState<AShooterGameState>();
	if (!IsValid(GameState)) return false;

	if (GameState->IsRoundGoing())
	{
		MessageLoggerComponent->AddErrorLoggerMessageSavingDuringRound();
		return false;
	}

	bool bSaved = GameState->SavePlayerCharacterState(this);
	if (bSaved) MessageLoggerComponent->AddLoggerMessageSavingSuccess();
	return bSaved;
}

bool APlayerCharacter::LoadProgress()
{
	auto GameState = GetWorld()->GetGameState<AShooterGameState>();
	if (!IsValid(GameState)) return false;
	return GameState->LoadPlayerCharacterState(this);
}

void APlayerCharacter::OnWeaponActionUnavailable_Implementation()
{
    Super::OnWeaponActionUnavailable_Implementation();

    PlayCancelActionSound();
}

void APlayerCharacter::ApplyAttachWeaponAction(bool bEquipAction)
{
    Super::ApplyAttachWeaponAction(bEquipAction);

    UpdateActiveWeaponHUD();
}

void APlayerCharacter::SwapReservedWeapon()
{
    Super::SwapReservedWeapon();

    UpdateActiveWeaponHUD();
}

bool APlayerCharacter::RefillAmmo(EWeaponType WeaponType)
{
	bool bSuccess = Super::RefillAmmo(WeaponType);

	auto SetAmmo = [&] (int32 Weapon = 0, int32 Stock = 0) -> bool
	{
		if (ShooterHUD.IsValid()) ShooterHUD->SetAmmoTo(WeaponType, Weapon, Stock);
		return bSuccess;
	};
	if (!bSuccess) return SetAmmo();
	if (!IsValid(GetWeaponSlotsSystem())) return SetAmmo();
	auto Firearms = Cast<AFirearmsWeapon>(GetWeaponSlotsSystem()->GetWeaponFromSlot(WeaponType));
	if (!IsValid(Firearms)) return SetAmmo();
	if (!IsValid(GetStatsComponent())) return SetAmmo();
	int32 AmmoCount = GetStatsComponent()->GetAmmoCount(WeaponType);
	return SetAmmo(Firearms->GetAmmoCount(), AmmoCount);
}

bool APlayerCharacter::TryPerformShot(bool bCheckIfHasAmmo)
{
	bool bSuccess = Super::TryPerformShot(bCheckIfHasAmmo);

	if (!bSuccess) return bSuccess;
	UWeaponSlotsSystem* SlotsSystem = GetWeaponSlotsSystem();
	if (!IsValid(SlotsSystem)) return bSuccess;
	auto Weapon = Cast<AFirearmsWeapon>(SlotsSystem->GetActiveWeapon());
	if (!IsValid(Weapon)) return bSuccess;
	if (ShooterHUD.IsValid())
		ShooterHUD->SetAmmoTo(SlotsSystem->GetActiveWeaponSlot(), Weapon->GetAmmoCount());
	return bSuccess;
}

bool APlayerCharacter::SetFullAmmoForWeapon(EWeaponType WeaponType)
{
	bool bSuccess = Super::SetFullAmmoForWeapon(WeaponType);

	if (!bSuccess) return bSuccess;
	UWeaponSlotsSystem* SlotsSystem = GetWeaponSlotsSystem();
	UShooterStatsComponent* Stats = GetStatsComponent();
	if (!(IsValid(SlotsSystem) && IsValid(Stats))) return bSuccess;
	auto Weapon = Cast<AFirearmsWeapon>(SlotsSystem->GetWeaponFromSlot(WeaponType));
	if (!IsValid(Weapon)) return bSuccess;
	if (ShooterHUD.IsValid())
		ShooterHUD->SetAmmoTo(WeaponType, Weapon->GetAmmoCount(), Stats->GetAmmoCount(WeaponType));
	return bSuccess;
}

void APlayerCharacter::MoveAction(const FInputActionValue& Value)
{
	if (!IsValid(GetController())) return;

	// input is a Vector2D
	const auto MovementVector = Value.Get<FVector2D>();

	// find out which way is forward
	const FRotator Rotation = GetController()->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// get right vector 
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// add movement 
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void APlayerCharacter::LookAction(const FInputActionValue& Value)
{
	if (!IsValid(GetController())) return;

	// input is a Vector2D
	const auto LookAxisVector = Value.Get<FVector2D>();

	// add yaw and pitch input to controller
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void APlayerCharacter::SwitchWeaponAction(const FInputActionValue& Value)
{
	const auto SlotVector = Value.Get<FVector2D>();
    EWeaponType* Slot = WeaponSlotInputs.Find(SlotVector);
    if (Slot) SwitchWeapon(*Slot);
}

void APlayerCharacter::SwitchAimFocusAction()
{
	if (IsDead()) return;
	if (IsValid(GetCombatSystem())) GetCombatSystem()->ToggleAimFocus();
}

void APlayerCharacter::ReloadWeaponAction()
{
	TryPerformReload(false);
}

void APlayerCharacter::TogglePauseMenuAction()
{
	if (!ShooterHUD.IsValid()) return;
	auto PlayerController = GetController<AShooterPlayerController>();

	if (ShooterHUD->IsPauseMenuOpened())
	{
		ShooterHUD->ClosePauseMenu();
		if (IsValid(PlayerController))
			PlayerController->SetInteractionMode(EPlayerInteractionMode::GameplayInputAndNotPause);
	}
	else
	{
		ShooterHUD->OpenPauseMenu();
		if (IsValid(PlayerController))
			PlayerController->SetInteractionMode(EPlayerInteractionMode::UIInputAndPause);
	}
}

void APlayerCharacter::ToggleSkillsMenuAction()
{
	if (!ShooterHUD.IsValid() || IsDead()) return;
	auto PlayerController = GetController<AShooterPlayerController>();

	if (ShooterHUD->IsSkillsProgressionMenuOpened())
	{
		ShooterHUD->CloseSkillsProgressionMenu();
		if (IsValid(PlayerController))
			PlayerController->SetInteractionMode(EPlayerInteractionMode::GameplayInputAndNotPause);
	}
	else
	{
		ShooterHUD->OpenSkillsProgressionMenu();
		if (IsValid(PlayerController))
			PlayerController->SetInteractionMode(EPlayerInteractionMode::UIInputAndPause);
	}
}

void APlayerCharacter::ResetRoundAction()
{
	if (IsDead()) return;

	auto GameState = GetWorld()->GetGameState<AShooterGameState>();
	if (!IsValid(GameState)) return;
	GameState->StartRound();
}

void APlayerCharacter::QuickSaveAction()
{
	SaveProgress();
}

void APlayerCharacter::PlayCancelActionSound()
{
	if (!IsValid(CancelActionSound)) return;
	constexpr float Volume = 1;
	constexpr float Pitch = 1;
	UGameplayStatics::PlaySound2D(GetWorld(), CancelActionSound, Volume, Pitch);
}

void APlayerCharacter::UpdateHealthBar(float Update, float Current, float Before, EPointsModifierType Modifier)
{
    if (!ShooterHUD.IsValid()) return;
	constexpr bool bAsPercent = true;
	ShooterHUD->SetHealthValue(Current, GetHealthPoints(bAsPercent));
}

void APlayerCharacter::UpdateActiveWeaponHUD()
{
    if (ShooterHUD.IsValid() && IsValid(GetWeaponSlotsSystem()))
		ShooterHUD->SelectWeaponType(GetWeaponSlotsSystem()->GetActiveWeaponSlot());
}

void APlayerCharacter::HandleAimTrace()
{
	if (!ShooterHUD.IsValid()) return;
	if (!IsShooterComponentsValid()) return;
	const EAimAvailability AimAvailability = GetSkillsSystem()->GetAimAvailabilityOrDefault();
	if (AimAvailability == EAimAvailability::NoAim) return;
	if (IsDead()) return ShooterHUD->SetAimTargetDetected(false);

	bool bTraceEnabled = false;
	if (AimAvailability == EAimAvailability::AimTraceAlways)
		bTraceEnabled = true;
	else if (AimAvailability == EAimAvailability::AimTraceAimingOnly)
		bTraceEnabled = GetCombatSystem()->GetAimingState();

	ShooterHUD->SetAimTargetDetected(bTraceEnabled && TraceWeaponAim());
}
