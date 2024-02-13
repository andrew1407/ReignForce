// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillsCountWidget.generated.h"


class UTextBlock;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API USkillsCountWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetSkillsCount(int32 Skills);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skills", meta = (BindWidget))
	TObjectPtr<UTextBlock> Title;
};
