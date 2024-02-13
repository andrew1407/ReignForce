// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Weapons/WeaponType.h"
#include "ShooterStatsWidget.generated.h"


class UWeaponItemWidget;
class UFirearmsItemWidget;
class UHealthBarWidget;
class USkillsCountWidget;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UShooterStatsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UShooterStatsWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintPure)
	UWeaponItemWidget* GetWeaponItemOf(EWeaponType WeaponType) const;

	UFUNCTION(BlueprintPure)
	UFirearmsItemWidget* GetFirearmsItemOf(EWeaponType WeaponType) const;

	UFUNCTION(BlueprintPure)
	const TSet<EWeaponType>& GetWeaponSlots() const { return WeaponSlots; }

	UFUNCTION(BlueprintCallable)
	void SetWeaponItemVisibility(EWeaponType WeaponType, bool bState);

	UFUNCTION(BlueprintCallable)
	void SetActiveWeaponAs(EWeaponType WeaponType);

	UFUNCTION(BlueprintCallable)
	void SetAmmoTo(EWeaponType WeaponType, int32 WeaponAmmo = -1, int32 StockAmmo = -1);

	UFUNCTION(BlueprintCallable)
	void SetHealthValue(int32 Value, float Percent);

	UFUNCTION(BlueprintCallable)
	void SetSkillsCount(int32 Value);

	FORCEINLINE UHealthBarWidget* GetHealthBar() const { return HealthBar; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (BindWidget))
	TObjectPtr<UWeaponItemWidget> UnarmedItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (BindWidget))
	TObjectPtr<UWeaponItemWidget> MeleeItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (BindWidget))
	TObjectPtr<UFirearmsItemWidget> PistolItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (BindWidget))
	TObjectPtr<UFirearmsItemWidget> RifleItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Health", meta = (BindWidget))
	TObjectPtr<UHealthBarWidget> HealthBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Skill", meta = (BindWidget))
	TObjectPtr<USkillsCountWidget> SkillsCount;

private:
	UPROPERTY(BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = true))
	TSet<EWeaponType> WeaponSlots;
};
