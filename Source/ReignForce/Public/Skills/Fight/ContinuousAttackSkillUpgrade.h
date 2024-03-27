// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skills/ShooterSkillUpgrade.h"
#include "Weapons/WeaponType.h"
#include "ContinuousAttackSkillUpgrade.generated.h"


/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class REIGNFORCE_API UContinuousAttackSkillUpgrade : public UShooterSkillUpgrade
{
	GENERATED_BODY()

public:
	UContinuousAttackSkillUpgrade(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual bool UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter) override;
    virtual bool IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter) override;
    virtual FText GetDescription_Implementation(AShooterCharacter* ShooterCharacter) override;
	virtual bool CanUpgrade_Implementation(AShooterCharacter* ShooterCharacter) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rank")
	EWeaponType WeaponType;
};
