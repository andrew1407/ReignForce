// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Skills/StageProgress.h"
#include "SkillsProgressionWidget.generated.h"


class UButton;
class UPanelWidget;

class USkillDescriptionWidget;
class USkillsCountWidget;
class USkillUpgradeWidget;
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
class REIGNFORCE_API USkillsProgressionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
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

protected:
	virtual void NativeConstruct() override;

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

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TMap<USkillUpgradeWidget*, FSkillUpgradeStages> Skills;

	TWeakObjectPtr<UUpgradesProgressStateComponent> ProgressState;

	UFUNCTION()
	void UnlockUpgradeStageForSelectedSkill();

	UFUNCTION()
	void HandleOnSkillSelected(USkillUpgradeWidget* Skill, bool bState);

	bool CheckIfCanUpgrade(UShooterSkillUpgrade* SkillUpgrade, APlayerCharacter* Character);
	TPair<USkillUpgradeWidget*, UShooterSkillUpgrade*> GetCurrentSkillState(APlayerCharacter* Character);
	TPair<int32, EStageProgress> GetStagesStatus(const FSkillUpgradeStages& Stages, APlayerCharacter* Character) const;
};
