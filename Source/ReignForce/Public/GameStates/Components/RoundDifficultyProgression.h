// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RoundDifficultyProgression.generated.h"


USTRUCT(BlueprintType)
struct FDifficultyValueContainer
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bValueRange = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bValueRange", EditConditionHides, ClampMin = 0, UIMin = 0))
	int32 ValueMin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bValueRange", EditConditionHides, ClampMin = 0, UIMin = 0))
	int32 ValueMax = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bValueRange", EditConditionHides, ClampMin = 0, UIMin = 0))
	int32 Value = 0;

	int32 GetValueByMode() const
	{
		return bValueRange ? FMath::RandRange(ValueMin, ValueMax) : Value;
	}

	bool FitsByComparingValue(int32 Comparable) const
	{
		return bValueRange ? (ValueMin >= Comparable && Comparable <= ValueMax) : (Comparable == Value);
	}
};

USTRUCT(BlueprintType)
struct FDifficultyOptions
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDifficultyValueContainer NecessarySkills;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDifficultyValueContainer EnemiesToSpawn;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REIGNFORCE_API URoundDifficultyProgression : public UActorComponent
{
	GENERATED_BODY()

public:	
	URoundDifficultyProgression(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure)
	int32 GetEnemiesCountBySkills(int32 Skills) const;

	UFUNCTION(BlueprintPure)
	int32 GetSkillsRewardForRoundWin() const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Difficulty|Rounds", meta = (AllowPrivateAccess = true))
	TArray<FDifficultyOptions> EnemiesPerRoundByGainedSkills;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward", meta = (AllowPrivateAccess = true, ClampMin = 0, UIMin = 0))
	int32 SkillsRewardPerRound;
};
