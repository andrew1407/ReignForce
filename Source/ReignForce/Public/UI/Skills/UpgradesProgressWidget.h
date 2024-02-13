// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Skills/StageProgress.h"
#include "UpgradesProgressWidget.generated.h"


class UImage;
class UTextBlock;

USTRUCT(BlueprintType)
struct FUpgradesProgressStateStyle
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Image;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color;
};

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UUpgradesProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UUpgradesProgressWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable)
	void LoadProgressionState(bool bCompleted);

	UFUNCTION(BlueprintCallable)
	void SetUpgradesCurrentCount(int32 Value);

	UFUNCTION(BlueprintCallable)
	void SetUpgradesTotalCount(int32 Value);

	UFUNCTION(BlueprintCallable)
	void ProvideWithState(int32 Current, int32 Total);

	FORCEINLINE bool IsProgressionCompleted() const { return StageProgress == EStageProgress::Completed; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	TObjectPtr<UImage> Icon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	TObjectPtr<UTextBlock> UpgradesCurrent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	TObjectPtr<UTextBlock> UpgradesTotal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (BindWidget))
	TObjectPtr<UTextBlock> Splitter;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression", meta = (AllowPrivateAccess = true))
	FUpgradesProgressStateStyle ProgressStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression", meta = (AllowPrivateAccess = true))
	FUpgradesProgressStateStyle ProgressCompletedStyle;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EStageProgress StageProgress;
};
