// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Common/ReignForceActivatableWidget.h"
#include "UI/Skills/StageProgress.h"
#include "SkillsProgressionWidget.generated.h"


class UButton;
class UPanelWidget;

class USkillDescriptionWidget;
class USkillsCountWidget;
class USkillUpgradeWidget;
class USkillUpgradeConfirmationWidget;
class UShooterSkillUpgrade;
class UUpgradesProgressWidget;

class USkillsDataAsset;
class UUpgradesProgressStateComponent;
class APlayerCharacter;

USTRUCT(BlueprintType)
struct FSkillUpgradeStages
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UShooterSkillUpgrade>> Stages;
};

/**
 * 
 */
UCLASS()
class REIGNFORCE_API USkillsProgressionWidget : public UReignForceActivatableWidget
{
	GENERATED_BODY()

public:
	USkillsProgressionWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure)
	bool IsSkillsDataAssetValid() const;

	UFUNCTION(BlueprintPure)
	TArray<USkillUpgradeWidget*> GetSkillUpgrades() const;

	UFUNCTION(BlueprintCallable)
	void CloseAndClearDescription();

	UFUNCTION(BlueprintCallable)
	void SetSkillsAvailableCount(int32 Count);

	UFUNCTION(BlueprintCallable)
	void SetUpgradesProgressState(int32 Current, int32 Total);

	UFUNCTION(BlueprintCallable)
	void SetDescriptionVisibility(bool bVisible);

	UFUNCTION(BlueprintCallable)
	void FillDescription(USkillUpgradeWidget* Skill, const FSkillUpgradeStages& Stages);

	FORCEINLINE UButton* GetCloseMenuButton() const { return CloseMenuButton; }
	FORCEINLINE USkillDescriptionWidget* GetSkillDescription() const { return SkillDescription; }

	/** A on the upgrade button while a skill is focused (A upgrades the selected skill), B on the close button */
	virtual FKey GetGamepadPromptKey(const UButton* Button, bool bFocused) const override;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill", meta = (BindWidget))
	TObjectPtr<USkillsCountWidget> SkillsCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill", meta = (BindWidget))
	TObjectPtr<UUpgradesProgressWidget> UpgradesProgress;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill|sUpgrades", meta = (BindWidget))
	TObjectPtr<UPanelWidget> SkillsContainer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skills|Info", meta = (BindWidget))
	TObjectPtr<USkillDescriptionWidget> SkillDescription;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SkillsProgression", meta = (BindWidget))
	TObjectPtr<UButton> CloseMenuButton;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills|DataAssets", meta = (AllowPrivateAccess = true))
	TObjectPtr<USkillsDataAsset> SkillsDataAsset;

	/** Asked before every upgrade, over the blurred menu */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills|Upgrade", meta = (AllowPrivateAccess = true))
	TSubclassOf<USkillUpgradeConfirmationWidget> UpgradeConfirmationClass;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TMap<USkillUpgradeWidget*, FSkillUpgradeStages> Skills;

	TWeakObjectPtr<UUpgradesProgressStateComponent> ProgressState;

	UFUNCTION()
	void UnlockUpgradeStageForSelectedSkill();

	UFUNCTION()
	void HandleOnSkillSelected(USkillUpgradeWidget* Skill, bool bState);

	/** Opens the upgrade confirmation for the selected skill when its next stage can be upgraded */
	UFUNCTION()
	void RequestUpgradeConfirmation();

	UFUNCTION()
	void HandleSkillUpgradeRequested(USkillUpgradeWidget* Skill);

	UFUNCTION()
	void HandleUpgradeConfirmation(bool bAccepted);

	/** Grid navigation: left/right go through the skills in order (wrapping rows), up/down to the closest skill of the next row, the last row leads to the close button */
	UFUNCTION()
	UWidget* HandleSkillNavigation(EUINavigation Direction);

	/** Index of the focused skill, the selected one if no skill is focused */
	int32 GetCurrentSkillIndex(const TArray<USkillUpgradeWidget*>& SkillUpgrades) const;

	/** The skills grid integration tests navigate it directly (a headless session has no keyboard focus) */
	friend class FSkillsMenuIntegrationTests;

	bool CheckIfCanUpgrade(UShooterSkillUpgrade* SkillUpgrade, APlayerCharacter* Character);
	TPair<USkillUpgradeWidget*, UShooterSkillUpgrade*> GetCurrentSkillState(APlayerCharacter* Character);
	TPair<int32, EStageProgress> GetStagesStatus(const FSkillUpgradeStages& Stages, APlayerCharacter* Character) const;
	void PlayOnUpgradeSound(EStageProgress StageProgress);
};
