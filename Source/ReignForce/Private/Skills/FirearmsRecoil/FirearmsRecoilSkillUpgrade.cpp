// Fill out your copyright notice in the Description page of Project Settings.


#include "Skills/FirearmsRecoil/FirearmsRecoilSkillUpgrade.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"


UFirearmsRecoilSkillUpgrade::UFirearmsRecoilSkillUpgrade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    WeaponType = EWeaponType::None;
    Index = 0;
    BaseDescription = FText::FromString(TEXT("Reduce recoil by {0} times (from the previous)."));
}

bool UFirearmsRecoilSkillUpgrade::UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;

    const TMap<EWeaponType, FRecoilModifiersCollection>& WeaponRecoilModifiers = Info.SkillsProgression->WeaponRecoilModifiers;
    const FRecoilModifiersCollection* Recoil = WeaponRecoilModifiers.Find(WeaponType);
    if (!(Recoil && Recoil->Collection.IsValidIndex(Index))) return false;

    int32* Modifier = Info.SkillsSystem->RanksProgression.WeaponRecoilModifiers.Find(WeaponType);
    if (!Modifier) return false;
    *Modifier = Index;
    return true;
}

bool UFirearmsRecoilSkillUpgrade::IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;
    int32* CurrentPtr = Info.SkillsSystem->RanksProgression.WeaponRecoilModifiers.Find(WeaponType);
    if (!CurrentPtr) return false;
    return (*CurrentPtr) >= Index;
}

FText UFirearmsRecoilSkillUpgrade::GetDescription_Implementation(AShooterCharacter* ShooterCharacter)
{
    const FString ToReplace = TEXT("{0}");
    FString Description = BaseDescription.ToString();
    if (!Description.Contains(ToReplace)) return BaseDescription;
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return BaseDescription;

    const TMap<EWeaponType, FRecoilModifiersCollection>& WeaponRecoilModifiers = Info.SkillsProgression->WeaponRecoilModifiers;
    const FRecoilModifiersCollection* Recoil = WeaponRecoilModifiers.Find(WeaponType);
    if (!(Recoil && Recoil->Collection.IsValidIndex(Index))) return BaseDescription;

    const int32 PreviousIndex = Index - 1;
    if (!Recoil->Collection.IsValidIndex(PreviousIndex)) return BaseDescription;

    const FVector2D CurrentRecoil = Recoil->Collection[Index];
    bool bHasZero = (CurrentRecoil.X * CurrentRecoil.Y) == 0;
    if (bHasZero) return BaseDescription;
    const FVector2D IncreaseRate = Recoil->Collection[PreviousIndex] / CurrentRecoil;

    const FString ValuesBlock = FString::Printf(TEXT("(%.1f, %.1f)"), IncreaseRate.X, IncreaseRate.Y);
    Description.ReplaceInline(*ToReplace, *ValuesBlock, ESearchCase::CaseSensitive);
    return FText::FromString(Description);
}

bool UFirearmsRecoilSkillUpgrade::CanUpgrade_Implementation(AShooterCharacter* ShooterCharacter)
{
    bool bCanUpgrade = Super::CanUpgrade_Implementation(ShooterCharacter);
    if (!(bCanUpgrade && IsValid(ShooterCharacter))) return false;
    UShooterSkillsSystem* SkillsSystem = ShooterCharacter->GetSkillsSystem();
    return IsValid(SkillsSystem) && SkillsSystem->AvailableSlots.Contains(WeaponType);
}
