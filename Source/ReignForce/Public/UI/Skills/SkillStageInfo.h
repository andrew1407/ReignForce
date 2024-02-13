// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Skills/StageProgress.h"
#include "SkillStageInfo.generated.h"


class UTextBlock;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API USkillStageInfo : public UUserWidget
{
	GENERATED_BODY()

public:
	USkillStageInfo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure)
	FText GetDescription() const;

	UFUNCTION(BlueprintCallable)
	void SetDescription(const FText& Info, EStageProgress StageProgress = EStageProgress::Locked, int32 Order = 0);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage", meta = (BindWidget))
	TObjectPtr<UTextBlock> StageDescription;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage|Progress", meta = (AllowPrivateAccess = true))
	TMap<EStageProgress, FLinearColor> ProgressStatusColors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage|Progress", meta = (AllowPrivateAccess = true))
	FLinearColor ProgressStatusColorDefault;
};
