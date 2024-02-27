// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"

#include "ShooterCharacter/TeamID.h"
#include "ShooterCharacter/PointsModifierType.h"
#include "ShooterCharacter/RanksProgressionSource.h"

#include "Weapons/WeaponType.h"
#include "Weapons/ShooterState.h"

#include "ShooterCharacter.generated.h"


class UBoxComponent;

class UWeaponSlotsSystem;
class UCombatSystem;
class UShooterStatsComponent;
class UShooterSkillsSystem;
class UDamageTakerComponent;
class UReactionsComponent;

struct FDamageEvent;

class AShooterCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChanged, float, Update, float, Current, float, Before, EPointsModifierType, Modifier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDamageTaken, float, Damage, AActor*, Cause, EWeaponType, WeaponType, bool, IsLethal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerish, AShooterCharacter*, ShooterSelf, AActor*, Cause);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDamageApplied, float, Damage, AShooterCharacter*, ShooterSelf, AActor*, HitTaret, bool, IsLethal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatsDefined, bool, bSuccess);

UCLASS(Abstract)
class REIGNFORCE_API AShooterCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bInvincible;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	ETeamID TeamId;

	#pragma region EVENTS

	UPROPERTY(BlueprintAssignable, Category = "ShooterCharacter")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "ShooterCharacter")
	FOnDamageTaken OnDamageTaken;

	UPROPERTY(BlueprintAssignable, Category = "ShooterCharacter")
	FOnPerish OnPerish;

	UPROPERTY(BlueprintAssignable, Category = "ShooterCharacter")
	FOnDamageApplied OnDamageApplied;

	UPROPERTY(BlueprintAssignable, Category = "ShooterCharacter")
	FOnStatsDefined OnStatsDefined;

	#pragma endregion

	virtual void Jump() override;

	#pragma region STATS_GETTERS

	FORCEINLINE virtual FGenericTeamId GetGenericTeamId() const override { return static_cast<uint8>(TeamId); }

	UFUNCTION(BlueprintNativeEvent, Category = "ShooterCharacter|Aiming")
    bool GetAimingLook(FVector& Direction, FRotator& Rotator) const;

    virtual bool GetAimingLook_Implementation(FVector& Direction, FRotator& Rotator) const;

	UFUNCTION(BlueprintPure)
	virtual bool IsShooterComponentsValid() const;

	UFUNCTION(BlueprintPure)
	virtual FVector2D GetMovementDirection() const;

	UFUNCTION(BlueprintPure)
	virtual bool TraceWeaponAim() const;

	UFUNCTION(BlueprintPure)
	virtual float GetAimOffsetPitch() const;

	UFUNCTION(BlueprintPure)
	bool IsDead() const;

	UFUNCTION(BlueprintPure)
	EWeaponType GetSelectedWeaponType() const;

	UFUNCTION(BlueprintPure)
	float GetHealthPoints(bool bAsPercent = false) const;

	UFUNCTION(BlueprintPure)
	EShooterState GetCombatShooterState() const;

	#pragma endregion

	#pragma region STATS_SETTERS

	UFUNCTION(BlueprintCallable)
	virtual void SwitchWeapon(EWeaponType Slot);

	UFUNCTION(BlueprintCallable)
	virtual void StartAiming();

	UFUNCTION(BlueprintCallable)
	virtual void StopAiming();

	UFUNCTION(BlueprintCallable)
	virtual void StartAttacking();

	UFUNCTION(BlueprintCallable)
	virtual void StopAttacking();

	UFUNCTION(BlueprintCallable)
	virtual void OnDuringAttack();

	UFUNCTION(BlueprintCallable)
	virtual bool MakeShot(bool bWithAutoreload = true);

	UFUNCTION(BlueprintCallable)
	virtual bool TryReenerateHealth(float Delta);

	#pragma endregion

	#pragma region STATE_CONTROL

	UFUNCTION(BlueprintCallable)
	virtual void Perish(AActor* Cause = nullptr);

	UFUNCTION(BlueprintCallable)
	virtual bool RefillAmmo(EWeaponType WeaponType);

	UFUNCTION(BlueprintCallable)
	virtual void RefillAmmoForAllAvailableFirearms();

	UFUNCTION(BlueprintCallable)
	virtual bool SetFullAmmoForWeapon(EWeaponType WeaponType);

	UFUNCTION(BlueprintCallable)
	virtual void SetFullAmmoForAllAvailableFirearms();

	UFUNCTION(BlueprintCallable)
	virtual void ApplyAttachWeaponAction(bool bEquipAction);

	UFUNCTION(BlueprintCallable)
	virtual void SwapReservedWeapon();

	UFUNCTION(BlueprintCallable)
	virtual bool TryPerformShot(bool bCheckIfHasAmmo = true);

	UFUNCTION(BlueprintCallable)
	virtual bool TryPerformReload(bool bAutoReload = false);

	UFUNCTION(BlueprintCallable)
	virtual bool TryActivateMeleeCollision(bool bState, int32 Index = 0);

	UFUNCTION(BlueprintCallable)
	virtual bool TryAddWeapon(TSubclassOf<AWeapon> WeaponClass, EWeaponType Slot);

	#pragma endregion

	#pragma region REACTIONS

	UFUNCTION(BlueprintCallable)
	virtual void ReactToSeen();

	UFUNCTION(BlueprintCallable)
	virtual void ReactToHeard();

	UFUNCTION(BlueprintCallable)
	virtual void ReactToLostSight();

	UFUNCTION(BlueprintCallable)
	virtual void ReactJoyfully();

	UFUNCTION(BlueprintCallable)
	virtual void GruntOnAttack();

	#pragma endregion

	FORCEINLINE UWeaponSlotsSystem* GetWeaponSlotsSystem() const { return WeaponSlotsSystem; }
	FORCEINLINE UCombatSystem* GetCombatSystem() const { return CombatSystem; }
	FORCEINLINE UShooterStatsComponent* GetStatsComponent() const { return StatsComponent; }
	FORCEINLINE UShooterSkillsSystem* GetSkillsSystem() const { return SkillsSystem; }
	FORCEINLINE UDamageTakerComponent* GetDamageTakerComponent() const { return DamageTakerComponent; }
	FORCEINLINE UReactionsComponent* GetReactionsComponent() const { return ReactionsComponent; }

	FORCEINLINE bool GetShouldAttackState() const { return bShouldAttack; }
	FORCEINLINE ERanksProgressionSource GetRanksProgressionSource() const { return RanksProgressionSource; }

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bShouldAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	ERanksProgressionSource RanksProgressionSource;

	virtual void BeginDestroy() override;
	virtual void Landed(const FHitResult& Hit) override;

	UFUNCTION()
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	void DefineStats();

	UFUNCTION(BlueprintCallable)
	virtual void DefineDamageHitReactions();

	UFUNCTION(BlueprintNativeEvent, Category = "ShooterCharacter|Events")
    void OnWeaponActionUnavailable();

    virtual void OnWeaponActionUnavailable_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "ShooterCharacter|Events")
	void ReactOnDamageCaused(float Damage, EWeaponType WeaponType, AActor* HitTarget, int32 Index);

	virtual void ReactOnDamageCaused_Implementation(float Damage, EWeaponType WeaponType, AActor* HitTarget, int32 Index);

private:
	#pragma region COMPONENTS

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems", meta = (AllowPrivateAccess = true))
	TObjectPtr<UWeaponSlotsSystem> WeaponSlotsSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCombatSystem> CombatSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems", meta = (AllowPrivateAccess = true))
	TObjectPtr<UShooterStatsComponent> StatsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems", meta = (AllowPrivateAccess = true))
	TObjectPtr<UShooterSkillsSystem> SkillsSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems", meta = (AllowPrivateAccess = true))
	TObjectPtr<UDamageTakerComponent> DamageTakerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems", meta = (AllowPrivateAccess = true))
	TObjectPtr<UReactionsComponent> ReactionsComponent;

	#pragma endregion
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Fight", meta = (AllowPrivateAccess = true))
	TObjectPtr<UBoxComponent> RightFist;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Fight", meta = (AllowPrivateAccess = true))
	TObjectPtr<UBoxComponent> LeftFist;

	void DeactivateMelee();
};
