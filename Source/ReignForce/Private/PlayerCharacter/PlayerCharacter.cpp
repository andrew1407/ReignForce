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
#include "CommonInputSubsystem.h"

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
#include "Weapons/Types/MeleeWeapon.h"

#include "UI/ShooterHUD.h"
#include "UI/ShooterStatsWidget.h"
#include "UI/Skills/SkillsProgressionWidget.h"
#include "UI/Components/UIMessageLoggerComponent.h"

#include "GameStates/ShooterGameState.h"
#include "GameStates/Components/UpgradesProgressStateComponent.h"
#include "ReinforcementLearning/Interfaces/AICommandsStrategy.h"
#include "Settings/ReignForceGameUserSettings.h"


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

	MeleeSwingVibration = { .2f, .1f, false, true };
	MeleeHitVibration = { .75f, .18f, true, true };
	ShotVibration = { .6f, .08f, true, true };
	MinShotVibrationScale = .2f;

	// Taken damage: a light buzz for a bullet, a heavy rumble for a machete hit & a stronger longer one for a punch
	BulletDamageVibration = { .35f, .12f, false, true };
	MacheteDamageVibration = { .65f, .2f, true, true };
	FistDamageVibration = { .9f, .26f, true, true };
}

void APlayerCharacter::BeginPlay()
{
	// Call the base class
	Super::BeginPlay();

	// The handlers can be bound already (saved with the placed character), so they aren't added twice
	OnHealthChanged.AddUniqueDynamic(this, &APlayerCharacter::UpdateHealthBar);
	OnStatsDefined.AddUniqueDynamic(this, &APlayerCharacter::HandleStatsDefined);

    auto PlayerController = GetController<APlayerController>();
	if (IsValid(PlayerController))
	{
    	//Add Input Mapping Context
		auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		// Registered with the user settings for the player remapped keys to be applied
		FModifyContextOptions ContextOptions;
		ContextOptions.bNotifyUserSettings = true;
		if (IsValid(Subsystem) && Inputs.MappingContext) Subsystem->AddMappingContext(Inputs.MappingContext, 0, ContextOptions);
	
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
	if (const FGamepadVibration* Vibration = FindDamageVibration(GetDamageVibrationSource(DamageCauser))) PlayVibration(*Vibration);
	bool bPerished = IsValid(GetStatsComponent()) && GetStatsComponent()->HasNoHealth();
	if (bPerished) Perish(DamageCauser);
	return DamageTaken;
}

EDamageVibrationSource APlayerCharacter::GetDamageVibrationSource(const AActor* DamageCauser)
{
	if (!IsValid(DamageCauser)) return EDamageVibrationSource::None;
	if (DamageCauser->IsA<AFirearmsWeapon>()) return EDamageVibrationSource::Bullet;
	if (DamageCauser->IsA<AMeleeWeapon>()) return EDamageVibrationSource::Machete;
	// Punches are dealt by the character itself (its fists aren't separate actors)
	if (DamageCauser->IsA<AShooterCharacter>()) return EDamageVibrationSource::Fist;
	return EDamageVibrationSource::None;
}

const FGamepadVibration* APlayerCharacter::FindDamageVibration(EDamageVibrationSource Source) const
{
	switch (Source)
	{
		case EDamageVibrationSource::Bullet: return &BulletDamageVibration;
		case EDamageVibrationSource::Machete: return &MacheteDamageVibration;
		case EDamageVibrationSource::Fist: return &FistDamageVibration;
		default: return nullptr;
	}
}

void APlayerCharacter::HandleStatsDefined(bool bSuccess)
{
	if (!bSuccess) return;

	auto GameState = GetWorld()->GetGameState<AShooterGameState>();
	if (IsValid(GameState) && IsValid(GetWeaponSlotsSystem()))
	{
		const EWeaponType SavedSlot = GameState->GetWeaponSlotFromLastSave();
		GetWeaponSlotsSystem()->AttachToActiveDirectly(SavedSlot);
	}

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

	if (Inputs.CycleWeaponAction)
		EnhancedInputComponent->BindAction(Inputs.CycleWeaponAction, ETriggerEvent::Started, this, &APlayerCharacter::CycleWeaponAction);
	
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

bool APlayerCharacter::CanSaveProgress() const
{
	if (IsDead()) return false;
	const UWorld* World = GetWorld();
	auto GameState = IsValid(World) ? World->GetGameState<AShooterGameState>() : nullptr;
	return IsValid(GameState) && !GameState->IsRoundGoing();
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
	auto LookAxisVector = Value.Get<FVector2D>();
	const UReignForceGameUserSettings* Settings = UReignForceGameUserSettings::GetReignForceGameUserSettings();
	if (IsValid(Settings))
	{
		LookAxisVector *= Settings->GetLookSensitivity();
		// The stick & the mouse are inverted separately: the input is from the device used last (CommonUI input type)
		auto PlayerController = Cast<APlayerController>(GetController());
		const UCommonInputSubsystem* InputSubsystem = IsValid(PlayerController) ? UCommonInputSubsystem::Get(PlayerController->GetLocalPlayer()) : nullptr;
		const bool bGamepad = InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;
		LookAxisVector = Settings->ApplyLookInversion(LookAxisVector, bGamepad);
	}

	// add yaw and pitch input to controller
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void APlayerCharacter::SwitchWeaponAction(const FInputActionValue& Value)
{
	if (!GetIsStatsDefined()) return;

	const auto SlotVector = Value.Get<FVector2D>();
    EWeaponType* Slot = WeaponSlotInputs.Find(SlotVector);
    if (Slot) SwitchWeapon(*Slot);
}

void APlayerCharacter::CycleWeaponAction(const FInputActionValue& Value)
{
	if (!GetIsStatsDefined() || !IsShooterComponentsValid()) return;

	const float Direction = Value.Get<float>();
	if (FMath::IsNearlyZero(Direction)) return;
	constexpr bool bClockwise = true;
	const EWeaponType Slot = FindNextWeaponSlot(Direction > 0 ? bClockwise : !bClockwise);
	if (Slot != GetWeaponSlotsSystem()->GetActiveWeaponSlot()) SwitchWeapon(Slot);
}

EWeaponType APlayerCharacter::FindNextWeaponSlot(bool bClockwise) const
{
	// Same order as the slots cross (arrows): up - fists, right - rifle, down - pistol, left - machete
	static const EWeaponType Order[] = { EWeaponType::Unarmed, EWeaponType::Rifle, EWeaponType::Pistol, EWeaponType::Melee };
	constexpr int32 Count = UE_ARRAY_COUNT(Order);

	const EWeaponType Active = GetWeaponSlotsSystem()->GetActiveWeaponSlot();
	int32 Current = 0;
	for (int32 i = 0; i < Count; ++i)
		if (Order[i] == Active) Current = i;

	// Locked (not unlocked by skills) or empty slots are skipped
	const int32 Step = bClockwise ? 1 : -1;
	for (int32 Offset = 1; Offset < Count; ++Offset)
	{
		const EWeaponType Slot = Order[(Current + Step * Offset + Count) % Count];
		bool bAvailable = Slot == EWeaponType::Unarmed
			|| (GetSkillsSystem()->AvailableSlots.Contains(Slot) && !GetWeaponSlotsSystem()->HasFreeSlot(Slot));
		if (bAvailable) return Slot;
	}
	return Active;
}

void APlayerCharacter::SwitchAimFocusAction()
{
	if (IsDead()) return;
	if (IsValid(GetCombatSystem())) GetCombatSystem()->ToggleAimFocus();
}

void APlayerCharacter::ReloadWeaponAction()
{
	if (!GetIsStatsDefined()) return;

	TryPerformReload(false);
}

void APlayerCharacter::TogglePauseMenuAction()
{
	if (!ShooterHUD.IsValid()) return;
	auto PlayerController = GetController<AShooterPlayerController>();

	// Pause input closes an opened skills menu instead of stacking the pause one over it
	if (ShooterHUD->IsSkillsProgressionMenuOpened())
	{
		ShooterHUD->CloseSkillsProgressionMenu();
		if (IsValid(PlayerController))
			PlayerController->SetInteractionMode(EPlayerInteractionMode::GameplayInputAndNotPause);
	}
	else if (ShooterHUD->IsPauseMenuOpened())
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
	if (ShooterHUD->IsPauseMenuOpened()) return;
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

bool APlayerCharacter::TryActivateMeleeCollision(bool bState, int32 Index)
{
	bool bActivated = Super::TryActivateMeleeCollision(bState, Index);
	// Every punch & machete swing is felt slightly
	if (bActivated && bState) PlayVibration(MeleeSwingVibration);
	return bActivated;
}

void APlayerCharacter::ReactOnDamageCaused_Implementation(float Damage, EWeaponType WeaponType, AActor* HitTarget, int32 Index)
{
	Super::ReactOnDamageCaused_Implementation(Damage, WeaponType, HitTarget, Index);

	bool bMeleeHit = WeaponType == EWeaponType::Unarmed || WeaponType == EWeaponType::Melee;
	if (bMeleeHit && Cast<AShooterCharacter>(HitTarget)) PlayVibration(MeleeHitVibration);
}

void APlayerCharacter::OnShotFired(const FVector2D& Recoil, const FVector2D& WeaponRecoil)
{
	Super::OnShotFired(Recoil, WeaponRecoil);

	// Reduced recoil (upgrades, aiming) gives a quieter shot rumble
	const float WeaponKick = WeaponRecoil.Size();
	const float RecoilScale = WeaponKick > KINDA_SMALL_NUMBER ? Recoil.Size() / WeaponKick : 1;
	PlayVibration(ShotVibration, FMath::Clamp(RecoilScale, MinShotVibrationScale, 1.f));
}

void APlayerCharacter::PlayVibration(const FGamepadVibration& Vibration, float Scale) const
{
	auto PlayerController = GetController<APlayerController>();
	if (!IsValid(PlayerController) || Vibration.Intensity <= 0 || Vibration.Duration <= 0) return;
	PlayerController->PlayDynamicForceFeedback(
		Vibration.Intensity * Scale,
		Vibration.Duration,
		Vibration.bLargeMotors, Vibration.bSmallMotors, // left
		Vibration.bLargeMotors, Vibration.bSmallMotors // right
	);
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
