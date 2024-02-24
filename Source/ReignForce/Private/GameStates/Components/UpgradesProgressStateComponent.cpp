// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/Components/UpgradesProgressStateComponent.h"


namespace
{
    constexpr uint32 MAX_SKILLS_COUINT = 35;
    constexpr uint32 MAX_UPGRADES_COUINT = 35;
}

UUpgradesProgressStateComponent::UUpgradesProgressStateComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    MaxSkillsAllowed = MAX_SKILLS_COUINT;
    Progression.SkillsAvailable = 0;

	MaxUpgrades = MAX_UPGRADES_COUINT;
	Progression.UpgradesUnlocked = 0;
}

bool UUpgradesProgressStateComponent::SetSkillsCount(int32 ToSet, bool bNotify)
{
    if (ToSet < 0) return false;
    const int32 ToSetChecked = FMath::Min(ToSet, MaxSkillsAllowed);
    const int32 BeforeSet = Progression.SkillsAvailable;
    Progression.SkillsAvailable = ToSetChecked;
    if (bNotify) OnSkillsAmountChanged.Broadcast(ESkillBalanceAction::Initialize, Progression.SkillsAvailable, ToSet, MaxSkillsAllowed, BeforeSet);
    return true;
}

bool UUpgradesProgressStateComponent::SetSkillsSpentCount(int32 ToSet)
{
    if (ToSet < 0) return false;
    Progression.SkillsSpent = FMath::Min(ToSet, MaxSkillsAllowed);
    return true;
}

bool UUpgradesProgressStateComponent::DropSkills(bool bNotify)
{
    if (Progression.SkillsAvailable == 0) return false;
    const int32 BeforeSet = Progression.SkillsAvailable;
    Progression.SkillsAvailable = 0;
    if (bNotify) OnSkillsAmountChanged.Broadcast(ESkillBalanceAction::Drop, Progression.SkillsAvailable, BeforeSet, MaxSkillsAllowed, BeforeSet);
    return true;
}

bool UUpgradesProgressStateComponent::AddSkills(int32 ToAdd, bool bNotify)
{
    if (ToAdd <= 0 || MaxSkillsAllowed == Progression.SkillsAvailable || IsFullOfSkills()) return false;
    const int32 BeforeSet = Progression.SkillsAvailable;
    const int32 ToAddFixed = FMath::Min(ToAdd, MaxSkillsAllowed - Progression.GetSkillsGained());
    Progression.SkillsAvailable += ToAddFixed;
    if (bNotify) OnSkillsAmountChanged.Broadcast(ESkillBalanceAction::Add, Progression.SkillsAvailable, ToAddFixed, MaxSkillsAllowed, BeforeSet);
    return true;
}

bool UUpgradesProgressStateComponent::TryTakeSkills(int32 ToTake, bool bNotify)
{
    if (!CanTake(ToTake)) return false;
    const int32 BeforeSet = Progression.SkillsAvailable;
    Progression.SkillsAvailable -= ToTake;
    if (bNotify) OnSkillsAmountChanged.Broadcast(ESkillBalanceAction::Take, Progression.SkillsAvailable, ToTake, MaxSkillsAllowed, BeforeSet);
    return true;
}

bool UUpgradesProgressStateComponent::AccumulateSpentSkills(int32 ToAdd)
{
    if (ToAdd <= 0 || Progression.GetSkillsGained() > MAX_SKILLS_COUINT) return false;
    Progression.SkillsSpent += ToAdd;
    return true;
}

bool UUpgradesProgressStateComponent::SetUpgradesCount(int32 Value, bool bNotify)
{
	if (Value < 0) return false;
    const int32 BeforeSet = Progression.UpgradesUnlocked;
	Progression.UpgradesUnlocked = FMath::Min(Value, MaxUpgrades);
    if (bNotify) OnUpgradesUnlocked.Broadcast(Progression.UpgradesUnlocked, Value, MaxUpgrades, BeforeSet);
	return true;
}

bool UUpgradesProgressStateComponent::AddUnlokedUpgrades(int32 Value, bool bNotify)
{
	if (Value < 0 || Progression.UpgradesUnlocked == MaxUpgrades) return false;
    const int32 BeforeSet = Progression.UpgradesUnlocked;
	Progression.UpgradesUnlocked = FMath::Min(Progression.UpgradesUnlocked + Value, MaxUpgrades);
    if (bNotify) OnUpgradesUnlocked.Broadcast(Progression.UpgradesUnlocked, Value, MaxUpgrades, BeforeSet);
	return true;
}
