// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skills/ShooterSkillUpgrade.h"
#include "UnlockAutoreloadSkillUpgrade.generated.h"


/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class REIGNFORCE_API UUnlockAutoreloadSkillUpgrade : public UShooterSkillUpgrade
{
	GENERATED_BODY()

public:
	UUnlockAutoreloadSkillUpgrade(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual bool UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter) override;
    virtual bool IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter) override;
	virtual bool CanUpgrade_Implementation(AShooterCharacter* ShooterCharacter) override;
};
