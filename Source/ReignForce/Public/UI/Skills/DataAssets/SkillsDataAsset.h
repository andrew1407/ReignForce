// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SkillsDataAsset.generated.h"


class USkillUpgradeWidget;
class UShooterSkillUpgrade;

USTRUCT(BlueprintType)
struct FSkillStages
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UShooterSkillUpgrade>> Collection;
};

/**
 * 
 */
UCLASS()
class REIGNFORCE_API USkillsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<TSubclassOf<USkillUpgradeWidget>, FSkillStages> PlayerSkills;
};
