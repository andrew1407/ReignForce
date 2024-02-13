// Fill out your copyright notice in the Description page of Project Settings.


#include "Skills/UnlockWeapon/UnlockSlotSkillUpgrade.h"
#include "Engine/AssetManager.h"
#include "GameFramework/PlayerController.h"

#include "GameModes/ReignForceGameMode.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"

#include "Weapons/Components/WeaponSlotsSystem.h"
#include "Weapons/Types/Weapon.h"
#include "UI/ShooterHUD.h"


UUnlockSlotSkillUpgrade::UUnlockSlotSkillUpgrade(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bAttachWeapon = true;
    ModelIndex = 0;
	WeaponType = EWeaponType::None;
    BaseDescription = FText::FromString(TEXT("Unlock weapon slot."));
}

bool UUnlockSlotSkillUpgrade::UpgradeShooter_Implementation(AShooterCharacter* ShooterCharacter)
{
    if (IsAlreadyUpgraded(ShooterCharacter)) return false;

    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;

    if (!bAttachWeapon)
    {
        Info.SkillsSystem->AvailableSlots.Add(WeaponType);
        RefreshState(ShooterCharacter, Info);
        return true;
    }

    auto GameMode = GetWorld()->GetAuthGameMode<AReignForceGameMode>();
    if (!IsValid(GameMode)) return false;

    TSoftClassPtr<AWeapon> WeaponClass = GameMode->GetWeaponModel(WeaponType, ModelIndex);
    if (WeaponClass.IsNull()) return false;

    UWeaponSlotsSystem* WeaponSlotsSystem = ShooterCharacter->GetWeaponSlotsSystem();
    if (!IsValid(WeaponSlotsSystem)) return false;
    Info.StatsComponent->SetWeaponModelOf(WeaponType, ModelIndex);
    Info.SkillsSystem->AvailableSlots.Add(WeaponType);

    UAssetManager::GetStreamableManager().RequestAsyncLoad(WeaponClass.ToSoftObjectPath(), [this, WeaponSlotsSystem, ShooterCharacter, WeaponClass, Info]
    {
        if (!(WeaponClass.IsValid() && IsValid(WeaponSlotsSystem))) return;
        bool bCreaterd = WeaponSlotsSystem->CreateAttachedWeapon(WeaponClass.Get(), WeaponType);
        if (!bCreaterd) return;
        RefreshState(ShooterCharacter, Info);
    });

    return true;
}

bool UUnlockSlotSkillUpgrade::IsAlreadyUpgraded_Implementation(AShooterCharacter* ShooterCharacter)
{
    FShooterSkillComponents Info = ExtractSkillComponents(ShooterCharacter);
    if (!Info.IsValid()) return false;
    return Info.SkillsSystem->AvailableSlots.Contains(WeaponType);
}

FText UUnlockSlotSkillUpgrade::GetDescription_Implementation(AShooterCharacter* ShooterCharacter)
{
    switch (WeaponType)
    {
        case EWeaponType::Melee: return FText::FromString(TEXT("Unlock melee slot."));
        case EWeaponType::Pistol: return FText::FromString(TEXT("Unlock pistol slot."));
        case EWeaponType::Rifle: return FText::FromString(TEXT("Unlock rifle slot."));
        default: return BaseDescription;
    }
}

void UUnlockSlotSkillUpgrade::RefreshState(AShooterCharacter* ShooterCharacter, const FShooterSkillComponents& Info)
{
    if (!(IsValid(ShooterCharacter) && Info.IsValid())) return;

    auto PlayerController = ShooterCharacter->GetController<APlayerController>();
    if (IsValid(PlayerController))
    {
        auto ShooterHUD = PlayerController->GetHUD<AShooterHUD>();
        if (IsValid(ShooterHUD)) ShooterHUD->SetVisibleSlots(Info.SkillsSystem->AvailableSlots);
    }

    bool bFirearms = UWeaponSlotsSystem::IsFirearmsType(WeaponType);
    if (bFirearms)
    {
        const int32 Ammo = Info.SkillsSystem->GetAmmoCapacityOrDefault(WeaponType);
        Info.StatsComponent->AddWeaponAmmo(WeaponType, Ammo);
        ShooterCharacter->RefillAmmo(WeaponType);
    }
}
