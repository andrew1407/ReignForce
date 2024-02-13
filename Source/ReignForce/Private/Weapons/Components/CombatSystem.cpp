// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Components/CombatSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Perception/AISense_Damage.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"

#include "Utils/Cooldown.h"
#include "Weapons/Types/Weapon.h"


// Sets default values for this component's properties
UCombatSystem::UCombatSystem(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ArmLengthDefault = 400;
	ArmLengthAiming = 170;
	AimingSpeed = 10;
	PunchAimDistance = 70;
	AimRecoilReducer = 1.5;
	PunchCooldown = .2f;

	bAiming = false;
	bAimFocus = false;
	ShooterState = EShooterState::Idle;
	RateOfFireToHandle = EWeaponType::None;

	RateOfFireDelays =
	{
		{ EWeaponType::Pistol, { FTimerHandle(), false } },
		{ EWeaponType::Rifle, { FTimerHandle(), false } },
	};

	PunchHitChance = 1;

	FistDamage = 1;
	RightFistIndex = 0;
	LeftFistIndex = 1;
}

// Called when the game starts
void UCombatSystem::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!IsValid(Owner)) return;

	AimingTimeline = Owner->GetComponentByClass<UTimelineComponent>();
	CameraBoom = Owner->GetComponentByClass<USpringArmComponent>();
	FollowCamera = Owner->GetComponentByClass<UCameraComponent>();

	if (CameraBoom.IsValid()) CameraBoom->TargetArmLength = ArmLengthDefault;

	if (IsValid(AimimgTimelineCurve) && AimingTimeline.IsValid())
	{
		FOnTimelineFloat TimelineCallback;
		TimelineCallback.BindDynamic(this, &UCombatSystem::OnAimingUpdate);
		AimingTimeline->AddInterpFloat(AimimgTimelineCurve, TimelineCallback);
	}

	for (UActorComponent* Component : Owner->GetComponents())
	{
		if (!Component->IsA(UBoxComponent::StaticClass())) continue;
		UBoxComponent* BoxComponent = Cast<UBoxComponent>(Component);
		bool bRightFist = !RightFistCollisionName.IsNone() && RightFistCollisionName == BoxComponent->GetName();
		bool bLeftFist = !LeftFistCollisionName.IsNone() && LeftFistCollisionName == BoxComponent->GetName();
		if (!(bRightFist || bLeftFist)) continue;
		TWeakObjectPtr<UBoxComponent>& Fist = bRightFist ? RightFist : LeftFist;
		Fist = BoxComponent;
		Fist->OnComponentBeginOverlap.AddDynamic(this, &UCombatSystem::OnPunch);
	}

	SetFistCollisionState(false, 0);
	SetFistCollisionState(false, 1);

	if (RightFist.IsValid()) RightFistCooldown = NewObject<UCooldown>(this);
	if (LeftFist.IsValid()) LeftFistCooldown = NewObject<UCooldown>(this);

	SetRotationLock(bAimFocus);
	if (bAimFocus) RotateYawToCameraView();
}

void UCombatSystem::ToggleAimFocus()
{
	bAimFocus = !bAimFocus;
	SetRotationLock(bAimFocus);
	if (bAimFocus) RotateYawToCameraView();
}

void UCombatSystem::SetRotationLock(bool bRotationLocked)
{
	auto Character = GetOwner<ACharacter>();
	if (!IsValid(Character)) return;
	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (!IsValid(MovementComponent)) return;
	MovementComponent->bOrientRotationToMovement = !bRotationLocked;
	MovementComponent->bUseControllerDesiredRotation = bRotationLocked;
}

void UCombatSystem::RotateYawToCameraView()
{
	auto Character = GetOwner<ACharacter>();
	if (!(IsValid(Character) && FollowCamera.IsValid())) return;
	FRotator CharacterRotation = Character->GetActorRotation();
	CharacterRotation.Yaw = FollowCamera->GetComponentRotation().Yaw;
	Character->SetActorRotation(CharacterRotation);
}

void UCombatSystem::ActivateAimingAction(bool bState)
{
	if (bState == bAiming) return;
	bAiming = bState;
	SetRotationLock(bState);
	if (!IsValid(AimimgTimelineCurve))
	{
		CameraBoom->TargetArmLength = bState ? ArmLengthAiming : ArmLengthDefault;
		return;
	}
	if (!AimingTimeline.IsValid()) return;
	if (AimingTimeline->IsPlaying()) AimingTimeline->Stop();
	if (bState) AimingTimeline->PlayFromStart();
	else AimingTimeline->Reverse();
}

void UCombatSystem::StartAiming(float Intensity)
{
	AimingSpeed = Intensity;
	RotateYawToCameraView();
	ActivateAimingAction(true);
}

void UCombatSystem::StopAiming()
{
	ActivateAimingAction(false);
	SetRotationLock(bAimFocus || ShooterState == EShooterState::Attacking);
}

bool UCombatSystem::TraceAimWith(EWeaponType WeaponType, AWeapon* Weapon, const FVector& Direction) const
{
	if (WeaponType == EWeaponType::None) return false;
	FHitResult HitResult;
	bool bAimTraced = WeaponType != EWeaponType::Unarmed
		? IsValid(Weapon) && Weapon->Aim(HitResult, Direction)
		: PunchAimTrace(HitResult, Direction);
	if (!bAimTraced) return false;
    AActor* HitActor = HitResult.GetActor();
	for (const auto& AimTarget : AimTargets)
	{
		if (HitActor->IsA(AimTarget)) return true;
	}
	return false;
}

void UCombatSystem::OnAimingUpdate(float Value)
{
	if (!CameraBoom.IsValid()) return;
	float Interpolated = FMath::Lerp(ArmLengthDefault, ArmLengthAiming, Value * AimingSpeed);
	float& Arm = CameraBoom->TargetArmLength;
	Arm = FMath::Clamp(Interpolated, ArmLengthAiming, ArmLengthDefault);
	if (!AimingTimeline.IsValid()) return;
	bool bCloseToAim = FMath::IsNearlyEqual(Arm, ArmLengthAiming);
	bool bCloseToDefault = FMath::IsNearlyEqual(Arm, ArmLengthDefault);
	bool bReachedBounds = (bAiming && bCloseToAim) || (!bAiming && bCloseToDefault);
	if (bReachedBounds && AimingTimeline->IsPlaying()) AimingTimeline->Stop();
}

bool UCombatSystem::AttackWith(EWeaponType Weapon, int32 Rank, float Rate)
{
	FAttackDescription* AttackDescription = AttackAnimations.Find(Weapon);
	if (!AttackDescription) return false;

	auto Character = GetOwner<ACharacter>();
	if (!IsValid(Character)) return false;
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!IsValid(Mesh)) return false;
	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	if (!IsValid(AnimInstance)) return false;

	TArray<UAnimMontage*>& Attacks = AttackDescription->Animations;
	if (!Attacks.IsValidIndex(Rank)) return false;
	UAnimMontage* MontageToPlay = Attacks[Rank];
	if (!IsValid(MontageToPlay)) return false;

	float MontageLength = AnimInstance->Montage_Play(MontageToPlay, Rate);
	return MontageLength != 0;
}

bool UCombatSystem::ReloadWeapon(EWeaponType Weapon, float Rate)
{
	UAnimMontage** ReloadAnimation = ReloadAnimations.Find(Weapon);
	if (!(ReloadAnimation && IsValid(*ReloadAnimation))) return false;

	auto Character = GetOwner<ACharacter>();
	if (!IsValid(Character)) return false;
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!IsValid(Mesh)) return false;
	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	if (!IsValid(AnimInstance)) return false;

	float MontageLength = AnimInstance->Montage_Play(*ReloadAnimation, Rate);
	return MontageLength != 0;
}

void UCombatSystem::ActivateRateOfFireDelay(EWeaponType Weapon, float Delay)
{
	TPair<FTimerHandle, bool>* Info = RateOfFireDelays.Find(Weapon);
	if (!Info) return;
	Info->Value = true;
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	RateOfFireToHandle = Weapon;
	TimerManager.SetTimer(Info->Key, this, &UCombatSystem::EnableDelayedShooting, Delay);
}

void UCombatSystem::StopRateOfFireDelay(EWeaponType Weapon)
{
	TPair<FTimerHandle, bool>* Info = RateOfFireDelays.Find(Weapon);
	if (!Info) return;
	GetWorld()->GetTimerManager().ClearTimer(Info->Key);
	Info->Value = false;
}

bool UCombatSystem::IsWeaponDelaying(EWeaponType Weapon) const
{
	const TPair<FTimerHandle, bool>* Info = RateOfFireDelays.Find(Weapon);
	return Info ? Info->Value : false;
}

UBoxComponent* UCombatSystem::GetFistCollision(int32 Index) const
{
	if (Index == RightFistIndex) return RightFist.Get();
	if (Index == LeftFistIndex) return LeftFist.Get();
	return nullptr;
}

bool UCombatSystem::PunchAimTrace(FHitResult& HitResult, const FVector& Direction) const
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner)) return false;
	const FVector CollisionLocation = Owner->GetActorLocation();
	const FVector End = CollisionLocation + Direction * PunchAimDistance;
    FCollisionQueryParams TraceParams(FName(TEXT("LineTrace")), false, Owner); // Additional parameters for the trace
	TraceParams.AddIgnoredActor(Owner);
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CollisionLocation, End, ECollisionChannel::ECC_GameTraceChannel1, TraceParams);
	return bHit;
}

void UCombatSystem::SetFistCollisionState(bool bState, int32 Index)
{
	UBoxComponent* Fist = GetFistCollision(Index);
	if (!IsValid(Fist)) return;
	ECollisionEnabled::Type Collision = bState ? ECollisionEnabled::Type::QueryAndPhysics : ECollisionEnabled::Type::NoCollision;
	Fist->SetCollisionEnabled(Collision);
}

void UCombatSystem::EnableDelayedShooting()
{
	TPair<FTimerHandle, bool>* Info = RateOfFireDelays.Find(RateOfFireToHandle);
	if (!Info) return;
	Info->Value = false;
	RateOfFireToHandle = EWeaponType::None;
}

void UCombatSystem::OnPunch(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AActor* Owner = GetOwner();
	if (Owner == OtherActor) return;

	bool bRightFist = RightFist == OverlappedComponent;
	if (!(bRightFist || LeftFist == OverlappedComponent)) return;
	TObjectPtr<UCooldown>& Cooldown = bRightFist ? RightFistCooldown : LeftFistCooldown;
	if (PunchHitChance < FMath::FRand()) return;
	if (PunchCooldown != 0 && IsValid(Cooldown))
	{
		if (Cooldown->HasCooldown(OtherActor)) return;
        Cooldown->Activate(OtherActor, PunchCooldown);
	}

	float Damage = OtherActor->TakeDamage(FistDamage, FDamageEvent(), nullptr, Owner);
    const FName NoiseTag = *FString::Printf(TEXT("Punch.Melee.%s"), *GetNameSafe(OtherActor));
    UAISense_Damage::ReportDamageEvent(GetWorld(), OtherActor, Owner, FistDamage, OtherActor->GetActorLocation(), Owner->GetActorLocation(), NoiseTag);
	if (Damage == 0) return;

	OnPunchDamageCaused.Broadcast(Damage, EWeaponType::Unarmed, OtherActor, bRightFist ? RightFistIndex : LeftFistIndex);
	if (IsValid(OnPunchHitSound)) UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        OnPunchHitSound,
        OverlappedComponent->GetComponentLocation(),
        OverlappedComponent->GetComponentRotation(),
        .6f
    );
}
