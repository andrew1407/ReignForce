// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/Cooldown.h"


bool UCooldown::Activate(UObject* Target, float Duration, bool bApplyRefrash)
{
    bool bHasCooldown = HasCooldown(Target);
    if (bHasCooldown) return bApplyRefrash && Refresh(Target, Duration);
    UWorld* World = GetWorld();
    if (!World) return false;
    FTimerHandle& TimerHandle = ActiveCooldowns.Add(Target, FTimerHandle());
    // Weak lambda: a collected cooldown object never gets called back (raw this would dangle)
    auto ClearCooldown = FTimerDelegate::CreateWeakLambda(this, [this, Target] { ActiveCooldowns.Remove(Target); });
    World->GetTimerManager().SetTimer(TimerHandle, ClearCooldown, Duration, false);
    return false;
}

bool UCooldown::Refresh(UObject* Target, float Duration)
{
    if (!HasCooldown(Target)) return false;
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(ActiveCooldowns[Target]);
    ActiveCooldowns.Remove(Target);
    return Activate(Target, Duration);
}
