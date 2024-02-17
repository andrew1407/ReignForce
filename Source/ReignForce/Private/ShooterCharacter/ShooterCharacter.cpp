// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter/ShooterCharacter.h"
#include "Engine/DamageEvents.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"

#include "GameModes/ReignForceGameMode.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"

#include "ShooterCharacter/Stats/RanksProgression.h"
#include "ShooterCharacter/Stats/PointsAccumulator.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"

#include "ShooterCharacter/Components/DamageTakerComponent.h"
#include "ShooterCharacter/Components/ReactionsComponent.h"

#include "Weapons/ProjectileSpread.h"
#include "Weapons/Components/WeaponSlotsSystem.h"
#include "Weapons/Components/CombatSystem.h"

#include "Weapons/Types/Weapon.h"
#include "Weapons/Types/MeleeWeapon.h"
#include "Weapons/Types/FirearmsWeapon.h"

namespace
{
	constexpr uint16 DEFAULT_SHOOTER_HEALTH = 100;
	constexpr const TCHAR* LEFT_FIST_SHOOTER_SOCKET = TEXT("hand_l");
	constexpr const TCHAR* RIGHT_FIST_SHOOTER_SOCKET = TEXT("hand_r");
}

// Sets default values
AShooterCharacter::AShooterCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;

	GetCharacterMovement()->BrakingDecelerationWalking = 2000;
	GetCharacterMovement()->BrakingDecelerationFalling = 120;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	RightFist = CreateDefaultSubobject<UBoxComponent>(GET_MEMBER_NAME_CHECKED(AShooterCharacter, RightFist));
	RightFist->SetupAttachment(GetMesh(), RIGHT_FIST_SHOOTER_SOCKET);
	RightFist->SetBoxExtent({ 5, 5, 5 });

	LeftFist = CreateDefaultSubobject<UBoxComponent>(GET_MEMBER_NAME_CHECKED(AShooterCharacter, LeftFist));
	LeftFist->SetupAttachment(GetMesh(), LEFT_FIST_SHOOTER_SOCKET);
	LeftFist->SetBoxExtent({ 5, 5, 5 });

	WeaponSlotsSystem = CreateDefaultSubobject<UWeaponSlotsSystem>(GET_MEMBER_NAME_CHECKED(AShooterCharacter, WeaponSlotsSystem));
	CombatSystem = CreateDefaultSubobject<UCombatSystem>(GET_MEMBER_NAME_CHECKED(AShooterCharacter, CombatSystem));
	StatsComponent = CreateDefaultSubobject<UShooterStatsComponent>(GET_MEMBER_NAME_CHECKED(AShooterCharacter, StatsComponent));
	SkillsSystem = CreateDefaultSubobject<UShooterSkillsSystem>(GET_MEMBER_NAME_CHECKED(AShooterCharacter, SkillsSystem));
	DamageTakerComponent = CreateDefaultSubobject<UDamageTakerComponent>(GET_MEMBER_NAME_CHECKED(AShooterCharacter, DamageTakerComponent));
	ReactionsComponent = CreateDefaultSubobject<UReactionsComponent>(GET_MEMBER_NAME_CHECKED(AShooterCharacter, ReactionsComponent));

	TeamId = ETeamID::None;
	RanksProgressionSource = ERanksProgressionSource::Default;

	bShouldAttack = false;
	bInvincible = false;
}

void AShooterCharacter::Jump()
{
	Super::Jump();

	if (IsDead()) return;
	bool bCurrentlyFalling = GetMovementComponent()->IsFalling();
	if (!bCurrentlyFalling && IsValid(ReactionsComponent))
		ReactionsComponent->MakeJumpGruntSound(GetActorLocation(), GetActorRotation());
}

void AShooterCharacter::BeginDestroy()
{
	OnHealthChanged.Clear();
	OnDamageTaken.Clear();
	OnPerish.Clear();
	OnDamageApplied.Clear();

	if (IsShooterComponentsValid())
	{
		CombatSystem->OnPunchDamageCaused.RemoveDynamic(this, &AShooterCharacter::ReactOnDamageCaused);
		for (const auto& WeaponType : SkillsSystem->AvailableSlots)
		{
			if (WeaponType == EWeaponType::Unarmed || WeaponType == EWeaponType::None) continue;
			AWeapon* Weapon = WeaponSlotsSystem->GetWeaponFromSlot(WeaponType);
			if (IsValid(Weapon))
				Weapon->OnDamageCaused.RemoveDynamic(this, &AShooterCharacter::ReactOnDamageCaused);
		}
		WeaponSlotsSystem->RemoveAllWeapons();
	}

	Super::BeginDestroy();
}

void AShooterCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (!IsValid(DamageTakerComponent) || IsDead()) return;
	float FallSpeed = -GetVelocity().Z;
	if (FallSpeed < 0) return;
	float DamagePercentage = 0;
	float Dmg = DamageTakerComponent->ApplyFallDamage(DamagePercentage, FallSpeed, Hit.GetActor());
	DamagePercentage = FMath::Clamp(DamagePercentage, 0, 1);
	if (IsValid(ReactionsComponent))
		ReactionsComponent->MakeOnLandedSound(Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), DamagePercentage);
}

bool AShooterCharacter::GetAimingLook_Implementation(FVector& Direction, FRotator& Rotator) const
{
	bool bComponentsValid = IsValid(GetMesh()) && IsValid(GetCapsuleComponent());
	if (bComponentsValid)
	{
		Direction = GetMesh()->GetRightVector();
		Rotator = GetCapsuleComponent()->GetComponentTransform().Rotator();
	}
	return bComponentsValid;
}

bool AShooterCharacter::IsShooterComponentsValid() const
{
	return IsValid(WeaponSlotsSystem) && IsValid(CombatSystem) && IsValid(SkillsSystem) && IsValid(StatsComponent);
}

FVector2D AShooterCharacter::GetMovementDirection() const
{
	const FVector Velocity = GetVelocity();
	const float ForwardSpeed = Velocity.Dot(GetActorForwardVector());
	const float RightSpeed = Velocity.Dot(GetActorRightVector());
	const float MaxSpeed = GetCharacterMovement()->MaxWalkSpeed;
	return FVector2D(ForwardSpeed, RightSpeed) / MaxSpeed;
}

void AShooterCharacter::SwitchWeapon(EWeaponType Slot)
{
	if (IsDead() || !IsShooterComponentsValid()) return;
	EShooterState& ShooterState = CombatSystem->ShooterState;
	if (ShooterState != EShooterState::Idle) return;
	EWeaponType ActiveWeapon = WeaponSlotsSystem->GetActiveWeaponSlot();
	bool bFirearms = WeaponSlotsSystem->IsFirearmsActive();
	bool bCancelled = Slot != EWeaponType::Unarmed && WeaponSlotsSystem->HasFreeSlot(Slot);
	if (bCancelled) return OnWeaponActionUnavailable();
	ShooterState = EShooterState::SwitchingWeapon;
	EWeaponType SwapTarget = Slot == ActiveWeapon ? EWeaponType::Unarmed : Slot;
	float SwappingRate = SkillsSystem->GetSwappingRateOrDefault();

	bool IsAbleToSwap = WeaponSlotsSystem->SwapActiveWeapon(SwapTarget, SwappingRate);
	if (!IsAbleToSwap)
	{
		ShooterState = EShooterState::Idle;
		return;
	}
	if (bFirearms) CombatSystem->StopRateOfFireDelay(ActiveWeapon);
}

bool AShooterCharacter::TraceWeaponAim() const
{
	if (!IsShooterComponentsValid()) return false;
	if (CombatSystem->ShooterState == EShooterState::SwitchingWeapon) return false;
	FVector AimDirection;
	FRotator AimRotator;
	if (!GetAimingLook(AimDirection, AimRotator)) return false;
	EWeaponType Slot = WeaponSlotsSystem->GetActiveWeaponSlot();
	AWeapon* Weapon = WeaponSlotsSystem->GetActiveWeapon();
	bool bAimTraced = CombatSystem->TraceAimWith(Slot, Weapon, AimDirection);
	return bAimTraced;
}

float AShooterCharacter::GetAimOffsetPitch() const
{
    float Pitch = GetBaseAimRotation().Pitch;
	constexpr int32 MaxDegrees = 360;
	constexpr int32 HalfDegrees = 180;
	return (Pitch < HalfDegrees) ? Pitch : (Pitch - MaxDegrees);
}

bool AShooterCharacter::IsDead() const
{
	return !IsValid(CombatSystem) || CombatSystem->ShooterState == EShooterState::Dead;
}

EWeaponType AShooterCharacter::GetSelectedWeaponType() const
{
	return IsValid(WeaponSlotsSystem) ? WeaponSlotsSystem->GetActiveWeaponSlot() : EWeaponType::None;
}

float AShooterCharacter::GetHealthPoints(bool bAsPercent) const
{
	if (!IsValid(StatsComponent)) return 0;
	return bAsPercent ? StatsComponent->GetCurrentHealthPercent() : StatsComponent->GetCurrentHealth();
}

EShooterState AShooterCharacter::GetCombatShooterState() const
{
	return IsValid(CombatSystem) ? CombatSystem->ShooterState : EShooterState::Idle;
}

void AShooterCharacter::DefineStats()
{
	auto OnFailed = [this] { OnStatsDefined.Broadcast(false); };
	if (!IsShooterComponentsValid()) return OnFailed();

	constexpr bool bSetFullHealth = true;
	StatsComponent->SetMaxHealth(SkillsSystem->GetHealthOrDefault(DEFAULT_SHOOTER_HEALTH), bSetFullHealth);
	const float SetHealth = SkillsSystem->GetHealthOrDefault(DEFAULT_SHOOTER_HEALTH);
	OnHealthChanged.Broadcast(SetHealth, SetHealth, -1, EPointsModifierType::DefaultInitialization);

	StatsComponent->HealthRegenerator.Speed = SkillsSystem->GetHealthRegenerationSpeedOrDefault(); 

	auto GameMode = GetWorld()->GetAuthGameMode<AReignForceGameMode>();
	if (!IsValid(GameMode)) return OnFailed();

	TArray<FSoftObjectPath> ModelPaths;
	TMap<TSoftClassPtr<AWeapon>, EWeaponType> LoadedModels;

	for (const auto& WeaponType : SkillsSystem->AvailableSlots)
	{
		if (WeaponType == EWeaponType::Unarmed || WeaponType == EWeaponType::None) continue;
		int32 WeaponModel = StatsComponent->GetWeaponModelOrDefault(WeaponType);
		StatsComponent->SetWeaponModelOf(WeaponType, WeaponModel);
		int32 ModelIndex = StatsComponent->GetWeaponModelOrDefault(WeaponType);
		TSoftClassPtr<AWeapon> WeaponClass = GameMode->GetWeaponModel(WeaponType, ModelIndex);

		if (!WeaponClass.IsNull())
		{
			ModelPaths.Add(WeaponClass.ToSoftObjectPath());
			LoadedModels.Add(WeaponClass, WeaponType);
		}
	}

	if (ModelPaths.IsEmpty()) return OnStatsDefined.Broadcast(true);

	UAssetManager::GetStreamableManager().RequestAsyncLoad(ModelPaths, [this, LoadedModels = MoveTemp(LoadedModels)]
	{
		for (const auto &[WeaponClass, WeaponType] : LoadedModels)
		{
			if (!(WeaponClass && WeaponClass.IsValid())) continue;
			bool bFirearms = UWeaponSlotsSystem::IsFirearmsType(WeaponType);
			bool bCreaterd = WeaponSlotsSystem->CreateAttachedWeapon(WeaponClass.Get(), WeaponType);
			if (!bFirearms) continue;
			const int32 Ammo = SkillsSystem->GetAmmoCapacityOrDefault(WeaponType);
			StatsComponent->AddWeaponAmmo(WeaponType, Ammo);
			RefillAmmo(WeaponType);
		}

		OnStatsDefined.Broadcast(true);
	});
}

void AShooterCharacter::DefineDamageHitReactions()
{
	if (!IsShooterComponentsValid()) return;

	CombatSystem->OnPunchDamageCaused.AddDynamic(this, &AShooterCharacter::ReactOnDamageCaused);
	for (const auto& WeaponType : SkillsSystem->AvailableSlots)
	{
		if (WeaponType == EWeaponType::Unarmed || WeaponType == EWeaponType::None) continue;
		AWeapon* Weapon = WeaponSlotsSystem->GetWeaponFromSlot(WeaponType);
		if (IsValid(Weapon))
			Weapon->OnDamageCaused.AddDynamic(this, &AShooterCharacter::ReactOnDamageCaused);
	}
}

float AShooterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageTaken = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (bInvincible || !IsValid(StatsComponent)) return 0;
	const float HealthBeforeSustraction = StatsComponent->GetCurrentHealth();
	bool bSubstracted = StatsComponent->SubstractHealthPoints(DamageTaken);
	if (!bSubstracted) return 0;
	
	OnHealthChanged.Broadcast(-DamageTaken, StatsComponent->GetCurrentHealth(), HealthBeforeSustraction, EPointsModifierType::Damage);

	bool bLethal = StatsComponent->HasNoHealth();
	EWeaponType WeaponType = EWeaponType::None;
	if (auto Weapon = Cast<AWeapon>(DamageCauser))
		WeaponType = Weapon->GetType();
	else if (auto Shooter = Cast<AShooterCharacter>(DamageCauser))
		WeaponType = Shooter->GetSelectedWeaponType();
	OnDamageTaken.Broadcast(DamageTaken, DamageCauser, WeaponType, bLethal);

	if (!bLethal && IsValid(DamageTakerComponent))
	{
		EWeaponType DamageType = DamageTakerComponent->DefineDamage(DamageCauser, AShooterCharacter::StaticClass());
		bool bScreamInPain = IsValid(ReactionsComponent) && ReactionsComponent->GenerateScreamInPainChance(DamageType);
		if (bScreamInPain)
		{
			UAudioComponent* Sound = ReactionsComponent->ScreamInPain(GetActorLocation(), FRotator::ZeroRotator, DamageType);
			ReactionsComponent->ResetReactionSound(Sound);
		}
	}
	return DamageTaken;
}

void AShooterCharacter::StartAiming()
{
	if (IsDead() || !IsShooterComponentsValid()) return;
	CombatSystem->StartAiming(SkillsSystem->GetAimingRateOrDefault());
}

void AShooterCharacter::StopAiming()
{
	if (IsValid(CombatSystem)) CombatSystem->StopAiming();
}

void AShooterCharacter::StartAttacking()
{
	if (IsDead()) return;

	bShouldAttack = true;
	if (!IsShooterComponentsValid()) return;
	if (!WeaponSlotsSystem->IsActiveWeaponValid()) return;
	EShooterState& ShooterState = CombatSystem->ShooterState;
	if (ShooterState != EShooterState::Idle) return;
	ShooterState = EShooterState::Attacking;

	bool bShouldRotate = !CombatSystem->GetAimingState() && WeaponSlotsSystem->IsFirearmsActive();
	if (bShouldRotate)
	{
		CombatSystem->SetRotationLock(true);
		CombatSystem->RotateYawToCameraView();
	}

	EWeaponType ActiveSlot = WeaponSlotsSystem->GetActiveWeaponSlot();
	bool bFirearms = WeaponSlotsSystem->IsFirearmsActive();
	TUniqueFunction<void()> ToIdleState = [&] { ShooterState = EShooterState::Idle; };
	if (WeaponSlotsSystem->IsFirearmsActive())
	{
		if (TryPerformShot(false)) return ToIdleState();
		if (CombatSystem->IsWeaponDelaying(ActiveSlot)) return ToIdleState();
	}
	const FWeaponSkillDescription WeaponInfo = SkillsSystem->GetWeaponAttackParamsOrDefault(ActiveSlot);
	bool bSuccess = CombatSystem->AttackWith(ActiveSlot, WeaponInfo.AnimationIndex, WeaponInfo.AnimationRate);
	if (!bSuccess) ToIdleState();
}

void AShooterCharacter::StopAttacking()
{
	bShouldAttack = false;
	if (!IsShooterComponentsValid()) return;
	if (!WeaponSlotsSystem->IsFirearmsActive()) return;
	EShooterState& ShooterState = CombatSystem->ShooterState;
	bool bAttacking = ShooterState == EShooterState::Attacking;
	bool bReloading = ShooterState == EShooterState::Reloading;
	if (!(bAttacking || bReloading)) return;
	if (bAttacking && !IsDead()) ShooterState = EShooterState::Idle;
	bool bAiming = CombatSystem->GetAimFocus() || CombatSystem->GetAimingState();
	if (!bAiming) CombatSystem->SetRotationLock(false);
}

void AShooterCharacter::OnDuringAttack()
{
	if (IsDead()) return;

	if (!bShouldAttack) return;
	if (!IsShooterComponentsValid()) return;
	if (!WeaponSlotsSystem->IsFirearmsActive()) return;
	EShooterState& ShooterState = CombatSystem->ShooterState;
	if (ShooterState == EShooterState::Idle) ShooterState = EShooterState::Attacking;
	if (ShooterState != EShooterState::Attacking) return;
	EWeaponType ActiveWeapon = WeaponSlotsSystem->GetActiveWeaponSlot();
	bool bContinuousInput = SkillsSystem->GetContituousAttackOrDefault(ActiveWeapon);
	const FWeaponSkillDescription WeaponInfo = SkillsSystem->GetWeaponAttackParamsOrDefault(ActiveWeapon);
	if (!bContinuousInput) return;
	if (CombatSystem->IsWeaponDelaying(ActiveWeapon)) return;
	if (TryPerformShot(false)) return;
	CombatSystem->AttackWith(ActiveWeapon, WeaponInfo.AnimationIndex, WeaponInfo.AnimationRate);
}

bool AShooterCharacter::MakeShot(bool bWithAutoreload)
{
	bool bSuccess = TryPerformShot(true);
	if (!(bWithAutoreload && bSuccess)) return bSuccess;
	bool bAutoreload = SkillsSystem->RanksProgression.bWeaponAutoReloadMode;
	return bAutoreload ? TryPerformReload(true) : true;
}

bool AShooterCharacter::TryReenerateHealth(float Delta)
{
	if (IsDead()) return false;
	bool bShouldReenerate = IsValid(StatsComponent) && StatsComponent->bHealthRegeneratorEnabled;
	if (!bShouldReenerate || StatsComponent->HasMaxHealth()) return false;
	FPointsAccumulator& Regenerator = StatsComponent->HealthRegenerator;

	if (IsValid(SkillsSystem))
		Regenerator.AccumulateCurrent(Delta, SkillsSystem->GetHealthRegenerationSpeedOrDefault());
	else
		Regenerator.AccumulateCurrent(Delta);

	if (Regenerator.ThresholdReached())
	{
		const float ToRegenerate = Regenerator.DropAccumulated();
		const float HealtheforeRegeneration = StatsComponent->GetCurrentHealth();
		bool bRegenerated = StatsComponent->AddHealthPoints(ToRegenerate);
		if (!bRegenerated) return false;
		OnHealthChanged.Broadcast(ToRegenerate, StatsComponent->GetCurrentHealth(), HealtheforeRegeneration, EPointsModifierType::Regeneration);
	}
	return true;
}

void AShooterCharacter::Perish(AActor* Cause)
{
	if (IsDead()) return;

	if (IsValid(GetMesh()))
	{
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (IsValid(AnimInstance))
		{
			UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
			constexpr float InBlendOutTime = 0;
			if (IsValid(AnimInstance)) AnimInstance->Montage_Stop(InBlendOutTime, CurrentMontage);
		}
	}

	if (IsValid(GetCapsuleComponent()))
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	}

	StopAttacking();
	DeactivateMelee();

	if (IsValid(ReactionsComponent))
		ReactionsComponent->MakePerishScream(GetActorLocation(), FRotator::ZeroRotator);

	if (IsValid(CombatSystem)) CombatSystem->ShooterState = EShooterState::Dead;
	OnPerish.Broadcast(this, Cause);
}

bool AShooterCharacter::RefillAmmo(EWeaponType WeaponType)
{
	bool bHasAvailableSlot = SkillsSystem->AvailableSlots.Contains(WeaponType);
	if (!bHasAvailableSlot) return false;

	bool bCanRefill = UWeaponSlotsSystem::IsFirearmsType(WeaponType) && IsShooterComponentsValid();
	if (!bCanRefill) return false;

	auto Weapon = Cast<AFirearmsWeapon>(WeaponSlotsSystem->GetWeaponFromSlot(WeaponType));
	if (!IsValid(Weapon)) return false;
	const int32 Ammo = StatsComponent->GetAmmoCount(WeaponType);
	const int32 Left = Weapon->RefillAmmo(Ammo, SkillsSystem->RanksProgression.bSaveAmmoMode);
	return StatsComponent->SetWeaponAmmo(WeaponType, Left);
}

void AShooterCharacter::RefillAmmoForAllAvailableFirearms()
{
	if (!IsValid(SkillsSystem)) return;

	for (const auto& WeaponType : SkillsSystem->AvailableSlots)
	{
		bool bFirearms = UWeaponSlotsSystem::IsFirearmsType(WeaponType);
		if (!bFirearms) continue;
		bool bRefilled = RefillAmmo(WeaponType);
	}
}

bool AShooterCharacter::SetFullAmmoForWeapon(EWeaponType WeaponType)
{
	if (!IsShooterComponentsValid()) return false;
	bool bHasAvailableSlot = SkillsSystem->AvailableSlots.Contains(WeaponType);
	if (!bHasAvailableSlot) return false;

	bool bFirearms = UWeaponSlotsSystem::IsFirearmsType(WeaponType);
	if (!bFirearms) return false;
	auto Weapon = Cast<AFirearmsWeapon>(WeaponSlotsSystem->GetWeaponFromSlot(WeaponType));
	if (!IsValid(Weapon)) return false;
	const int32 ToFillMax = StatsComponent->GetMaxAmmoCount(WeaponType);
	constexpr bool bSaveAmmoMode = false;
	Weapon->RefillAmmo(ToFillMax, bSaveAmmoMode);
	const int32 Ammo = FMath::Max(0, ToFillMax - Weapon->GetMaxAmmo());
	return StatsComponent->SetWeaponAmmo(WeaponType, Ammo);
}

void AShooterCharacter::SetFullAmmoForAllAvailableFirearms()
{
	for (const auto& WeaponType : SkillsSystem->AvailableSlots) SetFullAmmoForWeapon(WeaponType);
}

void AShooterCharacter::ApplyAttachWeaponAction(bool bEquipAction)
{
	if (!IsValid(WeaponSlotsSystem)) return;
	if (!bEquipAction)
		WeaponSlotsSystem->AttachWeaponToSlot(WeaponSlotsSystem->GetActiveWeaponSlot());
	else if (WeaponSlotsSystem->SetReservedToActive())
        WeaponSlotsSystem->AttachToActive(WeaponSlotsSystem->GetActiveWeaponSlot());
}

void AShooterCharacter::SwapReservedWeapon()
{
	if (IsDead() || !IsShooterComponentsValid()) return;
	EWeaponType ToEquip = WeaponSlotsSystem->GetReservedWeaponSlot();
    if (ToEquip != EWeaponType::Unarmed)
    {
        bool bCanEquip = WeaponSlotsSystem->EquipFromSlot(ToEquip, SkillsSystem->GetSwappingRateOrDefault());
        if (bCanEquip) return;
    }
    else
    {
        WeaponSlotsSystem->SetReservedToActive();
    }
    CombatSystem->ShooterState = EShooterState::Idle;
}

bool AShooterCharacter::TryPerformShot(bool bCheckIfHasAmmo)
{
	if (IsDead() || !IsShooterComponentsValid()) return false;
	auto Weapon = Cast<AFirearmsWeapon>(WeaponSlotsSystem->GetActiveWeapon());
	if (!IsValid(Weapon)) return false;
    EWeaponType WeaponType = WeaponSlotsSystem->GetActiveWeaponSlot();

	if (!(bCheckIfHasAmmo || Weapon->GetIsAmmoEmpty())) return false;

	FVector AimDirection;
	FRotator AimRotator;
	if (!GetAimingLook(AimDirection, AimRotator)) return false;
	const FProjectileSpread ProjectileSpread = SkillsSystem->GetFirearmsProjectileSpreadOrDefault(WeaponType);
	bool bWeaponFired = Weapon->Fire(AimDirection, AimRotator, ProjectileSpread);
	if (bWeaponFired)
	{
		FVector2D Recoil = Weapon->GetRecoil() * SkillsSystem->GetRecoilModifierOrDefault(WeaponType);
		if (CombatSystem->GetAimingState()) Recoil /= CombatSystem->GetAimRecoilReducer();
		AddControllerPitchInput(-Recoil.X);
		AddControllerYawInput(FMath::RandRange(-Recoil.Y, Recoil.Y));
	}

	CombatSystem->ActivateRateOfFireDelay(WeaponType, Weapon->GetRateOfFire());
	bool bContinuousInput = SkillsSystem->GetContituousAttackOrDefault(WeaponType);
	if (!bContinuousInput && CombatSystem->ShooterState == EShooterState::Attacking)
		CombatSystem->ShooterState = EShooterState::Idle;

	return true;
}

bool AShooterCharacter::TryPerformReload(bool bAutoReload)
{
	if (IsDead() || !IsShooterComponentsValid()) return false;
	if (!WeaponSlotsSystem->IsFirearmsActive()) return false;
	EShooterState& ShooterState = CombatSystem->ShooterState;
	if (ShooterState == EShooterState::Reloading) return false;
	EWeaponType ActiveWeapon = WeaponSlotsSystem->GetActiveWeaponSlot();
	if (StatsComponent->IsAmmoEmpty(ActiveWeapon))
	{
		if (!bAutoReload) OnWeaponActionUnavailable();
		return false;
	}
	auto Weapon = Cast<AFirearmsWeapon>(WeaponSlotsSystem->GetWeaponFromSlot(ActiveWeapon));
	if (!IsValid(Weapon) || Weapon->GetIsAmmoFull()) return false;
	if (bAutoReload && !Weapon->GetIsAmmoEmpty()) return false;
	float ReloadSpeed = SkillsSystem->GetWeaponReloadRateOrDefault();
	EShooterState PreviousState = ShooterState;
	ShooterState = EShooterState::Reloading;
	bool bSuccess = CombatSystem->ReloadWeapon(ActiveWeapon, ReloadSpeed);
	if (!bSuccess) ShooterState = PreviousState;
	return bSuccess;
}

bool AShooterCharacter::TryActivateMeleeCollision(bool bState, int32 Index)
{
	if (IsDead() || !IsShooterComponentsValid()) return false;
	EWeaponType WeaponType = WeaponSlotsSystem->GetActiveWeaponSlot();
    if (WeaponSlotsSystem->IsUnarmedActive())
    {
		CombatSystem->SetFistCollisionState(bState, Index);
		if (bState) CombatSystem->PunchHitChance = SkillsSystem->GetAttackHitChanceOrDefault(WeaponType);
		return true;
    }
	auto Weapon = Cast<AMeleeWeapon>(WeaponSlotsSystem->GetActiveWeapon());
	if (!IsValid(Weapon)) return false;
	if (bState) Weapon->HitChance = SkillsSystem->GetAttackHitChanceOrDefault(WeaponType);
	Weapon->SetCollisionState(bState, Index);
	return true;
}

void AShooterCharacter::ReactToSeen()
{
	if (!IsValid(ReactionsComponent)) return;
	UAudioComponent* Sound = ReactionsComponent->MakeWhenSeenSound(GetActorLocation(), FRotator::ZeroRotator);
	ReactionsComponent->ResetReactionSound(Sound);
}

void AShooterCharacter::ReactToHeard()
{
	if (!IsValid(ReactionsComponent)) return;
	UAudioComponent* Sound = ReactionsComponent->MakeWhenHeardSound(GetActorLocation(), FRotator::ZeroRotator);
	ReactionsComponent->ResetReactionSound(Sound);
}

void AShooterCharacter::ReactToLostSight()
{
	if (!IsValid(ReactionsComponent)) return;
	UAudioComponent* Sound = ReactionsComponent->MakeWhenLostSightSound(GetActorLocation(), FRotator::ZeroRotator);
	ReactionsComponent->ResetReactionSound(Sound);
}

void AShooterCharacter::ReactJoyfully()
{
	if (!IsValid(ReactionsComponent)) return;
	UAudioComponent* Sound = ReactionsComponent->MakeJoySound(GetActorLocation(), FRotator::ZeroRotator);
	ReactionsComponent->ResetReactionSound(Sound);
}

void AShooterCharacter::GruntOnAttack()
{
	if (!IsValid(ReactionsComponent)) return;
	UAudioComponent* Sound = ReactionsComponent->MakeAttackGrunt(GetActorLocation(), FRotator::ZeroRotator);
	ReactionsComponent->ResetReactionSound(Sound);
}

void AShooterCharacter::ReactOnDamageCaused_Implementation(float Damage, EWeaponType WeaponType, AActor* HitTarget, int32 Index)
{
	auto HitShooter = Cast<AShooterCharacter>(HitTarget);
	if (!IsValid(HitShooter)) return;
	bool bTargetDead = HitShooter->IsDead();
	if (bTargetDead)
	{
		ReactJoyfully();
		OnDamageApplied.Broadcast(Damage, this, HitTarget, bTargetDead);
	}
}

void AShooterCharacter::DeactivateMelee()
{
	if (!IsValid(WeaponSlotsSystem) || WeaponSlotsSystem->IsFirearmsActive()) return;
	EWeaponType WeaponType = WeaponSlotsSystem->GetActiveWeaponSlot();
	bool bUnarmed = IsValid(CombatSystem) && WeaponSlotsSystem->IsUnarmedActive();

	if (bUnarmed)
	{
		const auto& [LeftFistIndex, RightFistIndex] = CombatSystem->GetFistIndices();
		TryActivateMeleeCollision(false, LeftFistIndex);
		TryActivateMeleeCollision(false, RightFistIndex);
	}
	else
	{
		TryActivateMeleeCollision(false);
	}
}
