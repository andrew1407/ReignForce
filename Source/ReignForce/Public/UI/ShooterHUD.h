// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Weapons/WeaponType.h"
#include "GameStates/SkillBalanceAction.h"
#include "ShooterHUD.generated.h"


class UAimWidget;
class UShooterStatsWidget;
class USkillsProgressionWidget;
class UPauseMenuWidget;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API AShooterHUD : public AHUD
{
	GENERATED_BODY()

public:
	#pragma region AIM

	UFUNCTION(BlueprintCallable)
	bool CreateAimWidget();

	UFUNCTION(BlueprintCallable)
	void SetAimVisibility(bool bState);

	UFUNCTION(BlueprintCallable)
	void SetAimTargetDetected(bool bState);

	#pragma endregion

	#pragma region SHOOTER_STATS

	UFUNCTION(BlueprintCallable)
	bool CreateShooterStatsWidget();

	UFUNCTION(BlueprintCallable)
	void SelectWeaponType(EWeaponType WeaponType);

	UFUNCTION(BlueprintCallable)
	void SetVisibleSlots(const TSet<EWeaponType>& Slots);

	UFUNCTION(BlueprintCallable)
	void SetHealthValue(int32 Value, float Percent);

	UFUNCTION(BlueprintCallable)
	void SetAmmoTo(EWeaponType WeaponType, int32 WeaponAmmo = -1, int32 StockAmmo = -1);

	UFUNCTION(BlueprintCallable)
	void SetCurrentSkillsCountState(int32 Skills);

	UFUNCTION(BlueprintCallable)
	void SetCurrentUpgradesProgressionState(int32 Current, int32 Totla);

	#pragma endregion

	#pragma region SKILLS_PROGRESSION_MENU

	UFUNCTION(BlueprintPure)
	bool IsSkillsProgressionMenuOpened() const;

	UFUNCTION(BlueprintCallable)
	bool OpenSkillsProgressionMenu();

	UFUNCTION(BlueprintCallable)
	bool CloseSkillsProgressionMenu();

	#pragma endregion

	#pragma region SKILLS_PROGRESSION_MENU

	UFUNCTION(BlueprintPure)
	bool IsPauseMenuOpened() const;

	UFUNCTION(BlueprintCallable)
	bool OpenPauseMenu();

	UFUNCTION(BlueprintCallable)
	bool ClosePauseMenu();

	#pragma endregion

	FORCEINLINE UAimWidget* GetAimWidget() const { return AimWidget; }
	FORCEINLINE UShooterStatsWidget* GetShooterStatsWidget() const { return ShooterStatsWidget; }
	FORCEINLINE USkillsProgressionWidget* GetSkillsProgressionWidget() const { return SkillsProgressionWidget; }

protected:
	virtual void BeginPlay() override;

private:
	#pragma region AIM

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aim", meta = (AllowPrivateAccess = true))
	TSubclassOf<UAimWidget> AimWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Aim", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAimWidget> AimWidget;

	#pragma endregion

	#pragma region STATS

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = true))
	TSubclassOf<UShooterStatsWidget> ShooterStatsWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = true))
	TObjectPtr<UShooterStatsWidget> ShooterStatsWidget;

	#pragma endregion

	#pragma region MENUS

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu|Skills", meta = (AllowPrivateAccess = true))
	TSubclassOf<USkillsProgressionWidget> SkillsProgressionWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Menu|Skills", meta = (AllowPrivateAccess = true))
	TObjectPtr<USkillsProgressionWidget> SkillsProgressionWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu|Skills", meta = (AllowPrivateAccess = true))
	TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Menu|Pause", meta = (AllowPrivateAccess = true))
	TObjectPtr<UPauseMenuWidget> PauseMenuWidget;

	#pragma endregion

	UFUNCTION()
	void CloseSkillsMenuAction();

	UFUNCTION()
	void ClosePauseMenuAction();

	UFUNCTION()
	void UpdateSkillsCounter(ESkillBalanceAction SkillBalanceAction, int32 Current, int32 Updated, int32 Total, int32 Previous);

	UFUNCTION()
	void UpdateUnlockedProgress(int32 Current, int32 Updated, int32 Total, int32 Previous);

	void SetSkillsMenuMusicPlayState(bool bState);
	void SetPauseMenuMusicPlayState(bool bState);
};
