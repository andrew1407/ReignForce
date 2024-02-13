// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/Cooldown.h"


bool UCooldown::Activate(UObject* Target, float Duration, bool bApplyRefrash)
{
    bool bHasCooldown = HasCooldown(Target);
    if (bHasCooldown) return bApplyRefrash && Refresh(Target, Duration);
    FTimerHandle& TimerHandle = ActiveCooldowns.Add(Target, FTimerHandle());
    auto ClearCooldown = [this, Target] { ActiveCooldowns.Remove(Target); };
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, ClearCooldown, Duration, false);
    return false;
}

bool UCooldown::Refresh(UObject* Target, float Duration)
{
    if (!HasCooldown(Target)) return false;
    GetWorld()->GetTimerManager().ClearTimer(ActiveCooldowns[Target]);
    ActiveCooldowns.Remove(Target);
    return Activate(Target, Duration);
}
