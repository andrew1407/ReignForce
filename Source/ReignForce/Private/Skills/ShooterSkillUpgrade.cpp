// Fill out your copyright notice in the Description page of Project Settings.


#include "Skills/ShooterSkillUpgrade.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"


FShooterSkillComponents UShooterSkillUpgrade::ExtractSkillComponents(AShooterCharacter* ShooterCharacter) const
{
    FShooterSkillComponents Components;
    if (IsValid(ShooterCharacter))
    {
        Components.StatsComponent = ShooterCharacter->GetStatsComponent();
        Components.SkillsSystem = ShooterCharacter->GetSkillsSystem();
        if (Components.SkillsSystem.IsValid())
            Components.SkillsProgression = Components.SkillsSystem->GetProgressionAsset();
    }

    return MoveTemp(Components);
}
