// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "Logging/LogMacros.h"
#include "ShooterCharacter/CharacterInputActions.h"
#include "PlayerCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;

class UBoxComponent;
class UTimelineComponent;
class UInputComponent;
class USoundBase;

class USkillsBalance;
class AShooterHUD;

struct FInputActionValue;
struct FDamageEvent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 * 
 */
UCLASS()
class REIGNFORCE_API APlayerCharacter : public AShooterCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable)
	bool SaveProgress();

	UFUNCTION(BlueprintCallable)
	bool LoadProgress();

	virtual void Tick(float DeltaSeconds) override;
    virtual bool GetAimingLook_Implementation(FVector& Direction, FRotator& Rotator) const override;
	virtual void ApplyAttachWeaponAction(bool bEquipAction) override;
	virtual void SwapReservedWeapon() override;
	virtual bool RefillAmmo(EWeaponType WeaponType) override;
	virtual bool TryPerformShot(bool bCheckIfHasAmmo) override;
	virtual bool SetFullAmmoForWeapon(EWeaponType WeaponType) override;

	/** Returns CameraBoom subobject **/
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE const FCharacterInputActions& GetInputActions() const { return Inputs; }
	FORCEINLINE AShooterHUD* GetShooterHUD() const { return ShooterHUD.Get(); }

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	// To add mapping context
	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
    virtual void OnWeaponActionUnavailable_Implementation() override;

	UFUNCTION()
	void HandleStatsDefined(bool bSuccess);

private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = true))
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera|Aiming", meta = (AllowPrivateAccess = true))
	TObjectPtr<UTimelineComponent> AimingTimeline;

	UPROPERTY(BlueprintReadOnly, Category = "Refs", meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<AShooterHUD> ShooterHUD;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = true))
	FCharacterInputActions Inputs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|WeaponSlots", meta = (AllowPrivateAccess = true))
	TMap<FVector2D, EWeaponType> WeaponSlotInputs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> CancelActionSound;

	#pragma region ACTION_HANDLERS

	void MoveAction(const FInputActionValue& Value);
	void LookAction(const FInputActionValue& Value);

	void SwitchWeaponAction(const FInputActionValue& Value);
	void SwitchAimFocusAction();
	void ReloadWeaponAction();
	
	void TogglePauseMenuAction();
	void ToggleSkillsMenuAction();

	void ResetRoundAction();

	#pragma endregion

	UFUNCTION()
	void UpdateHealthBar(float Update, float Current, float Before, EPointsModifierType Modifier);

	void PlayCancelActionSound();
	void UpdateActiveWeaponHUD();
	void HandleAimTrace();
};
