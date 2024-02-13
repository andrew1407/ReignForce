#pragma once

#include "PlayerProgressionBalance.generated.h"


USTRUCT(BlueprintType)
struct REIGNFORCE_API FPlayerProgressionBalance
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
    int32 SkillsAvailable = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
    int32 SkillsSpent = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
    int32 UpgradesUnlocked = 0;

    FORCEINLINE int32 GetSkillsGained() const { return SkillsAvailable + SkillsSpent; }
};
