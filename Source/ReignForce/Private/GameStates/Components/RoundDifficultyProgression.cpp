// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/Components/RoundDifficultyProgression.h"


URoundDifficultyProgression::URoundDifficultyProgression(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    SkillsRewardPerRound = 3;
}

int32 URoundDifficultyProgression::GetEnemiesCountBySkills(int32 Skills) const
{

    for (const auto& Options : EnemiesPerRoundByGainedSkills)
    {
        bool bFitsBySkills = Options.NecessarySkills.FitsByComparingValue(Skills);
        if (bFitsBySkills) return Options.EnemiesToSpawn.GetValueByMode();
    }

    return 0;
}

int32 URoundDifficultyProgression::GetSkillsRewardForRoundWin() const
{
    return SkillsRewardPerRound;
}
