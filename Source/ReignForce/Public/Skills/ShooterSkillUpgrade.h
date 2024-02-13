// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ShooterSkillUpgrade.generated.h"


class AShooterCharacter;
class UShooterStatsComponent;
class UShooterSkillsSystem;
class UShooterSkillsProgression;

USTRUCT(BlueprintType)
struct FShooterSkillComponents
{
    GENERATED_USTRUCT_BODY()
	
    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<UShooterStatsComponent> StatsComponent;
    
    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<UShooterSkillsSystem> SkillsSystem;
    
    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<UShooterSkillsProgression> SkillsProgression;

    bool IsValid() const
    {
        return StatsComponent.IsValid() && SkillsSystem.IsValid() && SkillsProgression.IsValid();
    }
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class REIGNFORCE_API UShooterSkillUpgrade : public UObject
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure)
    FORCEINLINE int32 GetSkillCost() const { return Cost; }

	UFUNCTION(BlueprintNativeEvent, Category = "ShooterSkillUpgrade")
    bool UpgradeShooter(AShooterCharacter* ShooterCharacter);

    virtual bool UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter) { return false; }

    UFUNCTION(BlueprintNativeEvent, Category = "ShooterSkillUpgrade")
    bool IsAlreadyUpgraded(AShooterCharacter* ShooterCharacter);

    virtual bool IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter) { return false; }

    UFUNCTION(BlueprintNativeEvent, Category = "ShooterSkillUpgrade")
    FText GetDescription(AShooterCharacter* ShooterCharacter);

    virtual FText GetDescription_Implementation(AShooterCharacter* ShooterCharacter) { return BaseDescription; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Description")
    FText BaseDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Description", meta = (ClampMin = 1, UIMin = 1))
    int32 Cost = 1;

	UFUNCTION(BlueprintCallable)
    FShooterSkillComponents ExtractSkillComponents(AShooterCharacter* ShooterCharacter) const;
};
