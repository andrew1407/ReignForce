// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "EnemyCharacter.generated.h"


class UWidgetComponent;
class UHealthBarWidget;

struct FDamageEvent;

UCLASS()
class REIGNFORCE_API AEnemyCharacter : public AShooterCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	TArray<EWeaponType> WeaponSelectionPriorities;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

    virtual bool GetAimingLook_Implementation(FVector& Direction, FRotator& Rotator) const override;
	virtual float GetAimOffsetPitch() const override;

	virtual void Perish(AActor* Cause = nullptr) override;
	virtual bool MakeShot(bool bWithAutoreload = true) override;
	virtual void StartAttacking() override;
	virtual void OnDuringAttack() override;

	UFUNCTION(BlueprintCallable)
	void SetAimOffsetPitch(float Offset);

	UFUNCTION(BlueprintCallable)
	void SwitchWeaponToRandom(bool bAttachDirectly = false);

	UFUNCTION(BlueprintCallable)
	void SwitchWeaponByPriority(bool bAttachDirectly = false);

	UFUNCTION(BlueprintPure)
	FORCEINLINE UHealthBarWidget* GetHealthWidget() const { return HealthWidget.Get(); }

	FORCEINLINE UWidgetComponent* GetHealthWidgetComponent() const { return HealthWidgetComponent; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
	void HandleStatsDefined(bool bSuccess);

	UFUNCTION(BlueprintPure)
	EWeaponType GetWeaponSloatByPriority() const;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health|UI", meta = (AllowPrivateAccess = true))
	TObjectPtr<UWidgetComponent> HealthWidgetComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Health|UI", meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<UHealthBarWidget> HealthWidget;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bAttackOngoing;

	UPROPERTY()
	float AimOffsetPitch;

	UFUNCTION()
	void UpdateHealthBar(float Update, float Current, float Before, EPointsModifierType Modifier);

	void RotateHealthBarToPlayer();
	void CheckOutOfAmmo();
	void DisableSenses();
};
