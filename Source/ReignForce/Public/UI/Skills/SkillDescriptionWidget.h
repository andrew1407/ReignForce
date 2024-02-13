// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Skills/StageProgress.h"
#include "SkillDescriptionWidget.generated.h"


class UButton;
class UImage;
class UTextBlock;
class UPanelWidget;

class USkillStageInfo;

USTRUCT(BlueprintType)
struct FSkillStageDescriptionDetails
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Entries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EStageProgress StageProgress;
};

/**
 * 
 */
UCLASS()
class REIGNFORCE_API USkillDescriptionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USkillDescriptionWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Description")
	bool bStagesEnumerated;

	UFUNCTION(BlueprintCallable)
	void ClearDescription();

	UFUNCTION(BlueprintCallable)
	void SetTitle(const FName& SkillName, int32 StagesTotal, int32 CurrentStage);

	UFUNCTION(BlueprintCallable)
	void SetSkillImage(UTexture2D* Image);

	UFUNCTION(BlueprintCallable)
	void ClearStagesList();

	UFUNCTION(BlueprintCallable)
	void FillStagesList(const TArray<FSkillStageDescriptionDetails>& Info, bool bClearPreviously = true);

	UFUNCTION(BlueprintCallable)
	void SetSkillUpdateToSpend(int32 ToSpend = 1);

	UFUNCTION(BlueprintCallable)
	void SetUpgradeButtonVisibility(bool bState);

	UFUNCTION(BlueprintCallable)
	void SetUpgradeButtonEnabled(bool bState);

	UFUNCTION(BlueprintPure)
	bool GetUpgradeButtonEnabled() const;

	FORCEINLINE UButton* GetUpgradeSkillButton() const { return UpgradeSkillButton; }

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Description|Compoennts", meta = (BindWidget))
	TObjectPtr<UTextBlock> Title;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Description|Compoennts", meta = (BindWidget))
	TObjectPtr<UImage> SkillImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Description|Compoennts", meta = (BindWidget))
	TObjectPtr<UPanelWidget> StagesBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Description|Compoennts", meta = (BindWidget))
	TObjectPtr<UButton> UpgradeSkillButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Description|Compoennts", meta = (BindWidget))
	TObjectPtr<UTextBlock> UpgradeSkillButtonDescription;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Description|Stages", meta = (AllowPrivateAccess = true))
	TSubclassOf<USkillStageInfo> StageInfoClass;
};
