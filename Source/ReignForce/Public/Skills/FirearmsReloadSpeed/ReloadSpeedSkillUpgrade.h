// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skills/ShooterSkillUpgrade.h"
#include "ReloadSpeedSkillUpgrade.generated.h"


/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class REIGNFORCE_API UReloadSpeedSkillUpgrade : public UShooterSkillUpgrade
{
	GENERATED_BODY()

public:
	UReloadSpeedSkillUpgrade(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual bool UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter) override;
    virtual bool IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter) override;
    virtual FText GetDescription_Implementation(AShooterCharacter* ShooterCharacter) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rank", meta = (ClampMin = 0, UIMin = 0))
	int32 Index;
};
