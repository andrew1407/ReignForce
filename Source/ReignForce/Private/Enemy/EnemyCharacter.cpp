// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnemyCharacter.h"
#include "Engine/DamageEvents.h"
#include "Kismet/KismetMathLibrary.h"

#include "Sound/SoundBase.h"
#include "Camera/PlayerCameraManager.h"

#include "AIController.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"

#include "Weapons/Types/Weapon.h"
#include "Weapons/Types/FirearmsWeapon.h"
#include "Weapons/Components/WeaponSlotsSystem.h"
#include "Weapons/Components/CombatSystem.h"

#include "ShooterCharacter/TeamID.h"
#include "ShooterCharacter/RanksProgressionSource.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"

#include "UI/HealthBarWidget.h"


// Sets default values
AEnemyCharacter::AEnemyCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	HealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(GET_MEMBER_NAME_CHECKED(AEnemyCharacter, HealthWidgetComponent));
	HealthWidgetComponent->SetupAttachment(RootComponent);
	HealthWidgetComponent->SetCastShadow(false);

	GetCharacterMovement()->MaxWalkSpeed = 400;
	GetCharacterMovement()->BrakingDecelerationFalling = 120;
	GetStatsComponent()->bHealthRegeneratorEnabled = false;

	AimOffsetPitch = 0;
	TeamId = ETeamID::Enemy;
	bAttackOngoing = false;

	OnHealthChanged.AddDynamic(this, &AEnemyCharacter::UpdateHealthBar);

	WeaponSelectionPriorities =
	{
		EWeaponType::Rifle,
		EWeaponType::Pistol,
		EWeaponType::Melee,
		EWeaponType::Unarmed,
	};

	OnStatsDefined.AddDynamic(this, &AEnemyCharacter::HandleStatsDefined);
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsValid(GetController()))
	{
		auto AIController = GetWorld()->SpawnActor<AAIController>(AIControllerClass);
		if (IsValid(AIController)) AIController->Possess(this);
	}

	if (IsValid(HealthWidgetComponent))
		HealthWidget = Cast<UHealthBarWidget>(HealthWidgetComponent->GetUserWidgetObject());

	if (!IsShooterComponentsValid()) return;

	if (RanksProgressionSource == ERanksProgressionSource::Default) GetSkillsSystem()->InitializeDefaultRanks();
	DefineStats();
}

void AEnemyCharacter::HandleStatsDefined(bool bSuccess)
{
	if (!bSuccess) return;

	DefineDamageHitReactions();
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateHealthBarToPlayer();

	bool bShouldFire = IsValid(GetWeaponSlotsSystem()) && GetWeaponSlotsSystem()->IsFirearmsActive();
	if (bShouldFire) GetWorldTimerManager().SetTimerForNextTick(this, &AEnemyCharacter::OnDuringAttack);
}

float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageTaken = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (DamageTaken == 0) return DamageTaken;
	bool bPerished = IsValid(GetStatsComponent()) && GetStatsComponent()->HasNoHealth();
	if (bPerished) Perish();
	return DamageTaken;
}

EWeaponType AEnemyCharacter::GetWeaponSloatByPriority() const
{
	EWeaponType Current = GetSelectedWeaponType();
	if (!IsValid(GetWeaponSlotsSystem()) || WeaponSelectionPriorities.IsEmpty()) return Current;
	for (const auto& WeaponType : WeaponSelectionPriorities)
	{
		if (WeaponType == Current || WeaponType == EWeaponType::None) continue;
		if (WeaponType == EWeaponType::Unarmed) return WeaponType;
		AWeapon* Weapon = GetWeaponSlotsSystem()->GetWeaponFromSlot(WeaponType);
		if (!IsValid(Weapon)) continue;
		bool bFirearms = UWeaponSlotsSystem::IsFirearmsType(Weapon->GetType());
		if (bFirearms)
		{
			auto Firearms = Cast<AFirearmsWeapon>(Weapon);
			if (!IsValid(Firearms) || Firearms->GetIsAmmoEmpty()) continue;
		}
		return Weapon->GetType();
	}
	return EWeaponType::Unarmed;
}

void AEnemyCharacter::SetAimOffsetPitch(float Offset)
{
	// AimOffsetPitch = Offset;
	constexpr int32 MaxDegrees = 360;
	constexpr int32 HalfDegrees = 180;
	AimOffsetPitch = (Offset >= HalfDegrees) ? (Offset - MaxDegrees) : Offset;
}

float AEnemyCharacter::GetAimOffsetPitch() const
{
	// return (AimOffsetPitch >= 180) ? (AimOffsetPitch - 360) : AimOffsetPitch;
	return AimOffsetPitch;
}

bool AEnemyCharacter::GetAimingLook_Implementation(FVector& Direction, FRotator& Rotator) const
{
	bool bCapsuleComponentValid = IsValid(GetCapsuleComponent());
	if (bCapsuleComponentValid)
	{
		FRotator BaseRotation = GetCapsuleComponent()->GetComponentTransform().Rotator();
		BaseRotation.Pitch += GetAimOffsetPitch();
		Direction = BaseRotation.Vector();
		Rotator = BaseRotation;
	}
    return bCapsuleComponentValid;
}

void AEnemyCharacter::SwitchWeaponToRandom(bool bAttachDirectly)
{
	if (!IsValid(GetWeaponSlotsSystem())) return;
	const TArray<EWeaponType> Slots = GetWeaponSlotsSystem()->GetAvailableWeaponSlots();
	if (Slots.IsEmpty()) return;
	const EWeaponType ToSwitch = Slots[FMath::RandRange(0, Slots.Num() - 1)];
	if (bAttachDirectly)
	{
		if (IsValid(GetWeaponSlotsSystem()))
			GetWeaponSlotsSystem()->AttachToActiveDirectly(ToSwitch);
	}
	else
	{
		SwitchWeapon(ToSwitch);
	}
}

void AEnemyCharacter::SwitchWeaponByPriority(bool bAttachDirectly)
{
	EWeaponType ToSwitch = GetWeaponSloatByPriority();
	bool bCannotSwitch = ToSwitch == EWeaponType::None || ToSwitch == GetSelectedWeaponType();
	if (bCannotSwitch) ToSwitch = EWeaponType::Unarmed;
	if (bAttachDirectly)
	{
		if (IsValid(GetWeaponSlotsSystem()))
			GetWeaponSlotsSystem()->AttachToActiveDirectly(ToSwitch);
	}
	else
	{
		SwitchWeapon(ToSwitch);
	}
}

void AEnemyCharacter::RotateHealthBarToPlayer()
{
	if (!IsValid(HealthWidgetComponent)) return;
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!IsValid(PlayerController)) return;
	APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
	if (!IsValid(CameraManager)) return;
    const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(HealthWidgetComponent->GetComponentLocation(), CameraManager->GetCameraLocation());
	HealthWidgetComponent->SetWorldRotation(LookAtRotation);
}

void AEnemyCharacter::Perish(AActor* Cause)
{
	Super::Perish(Cause);

	if (IsValid(HealthWidgetComponent)) HealthWidgetComponent->SetVisibility(false);
	DisableSenses();
}

bool AEnemyCharacter::MakeShot(bool bWithAutoreload)
{
	bool bSuccess = Super::MakeShot(bWithAutoreload);

	CheckOutOfAmmo();
	return bSuccess;
}

void AEnemyCharacter::StartAttacking()
{
	if (bAttackOngoing) return;
	bAttackOngoing = true;
	Super::StartAttacking();
	bAttackOngoing = false;
}

void AEnemyCharacter::OnDuringAttack()
{
	if (bAttackOngoing) return;
	bAttackOngoing = true;
	Super::OnDuringAttack();
	bAttackOngoing = false;
}

void AEnemyCharacter::UpdateHealthBar(float Update, float Current, float Before, EPointsModifierType Modifier)
{
	if (!HealthWidget.IsValid()) return;
	constexpr bool bAsPercent = true;
	HealthWidget->SetHealthValue(Current, GetHealthPoints(bAsPercent));
}

void AEnemyCharacter::CheckOutOfAmmo()
{
	if (IsDead() || !IsShooterComponentsValid()) return;
	bool bReloading = GetCombatSystem()->ShooterState == EShooterState::Reloading;
	if (bReloading) return;
	bool IsAmmoEmpty = GetStatsComponent()->IsAmmoEmpty(GetWeaponSlotsSystem()->GetActiveWeaponSlot());
	if (!IsAmmoEmpty) return;
	auto Weapon = Cast<AFirearmsWeapon>(GetWeaponSlotsSystem()->GetActiveWeapon());
	if (!(IsValid(Weapon) && Weapon->GetIsAmmoEmpty())) return;

	StopAttacking();
	SwitchWeaponByPriority();
}

void AEnemyCharacter::DisableSenses()
{
	auto AIController = GetController<AAIController>();
	if (!IsValid(AIController)) return;
	UAIPerceptionComponent* PerceptionComponent = AIController->GetAIPerceptionComponent();
	if (!IsValid(PerceptionComponent)) return;
	PerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), false);
	PerceptionComponent->SetSenseEnabled(UAISense_Hearing::StaticClass(), false);
}
