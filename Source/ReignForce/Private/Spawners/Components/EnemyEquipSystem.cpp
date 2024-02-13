// Fill out your copyright notice in the Description page of Project Settings.


#include "Spawners/Components/EnemyEquipSystem.h"

#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

#include "ShooterCharacter/ShooterCharacter.h"
#include "ShooterCharacter/RanksProgressionSource.h"
#include "ShooterCharacter/Stats/RanksProgression.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"

#include "Enemy/ShooterAIController.h"
#include "PlayerCharacter/PlayerCharacter.h"
#include "Weapons/Components/WeaponSlotsSystem.h"


// Sets default values for this component's properties
UEnemyEquipSystem::UEnemyEquipSystem(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    constexpr bool bApplyExtraCheck = true;
    constexpr bool bIgnoreExtraCheck = false;

	WeaponEquipProbabilities =
    {
        { EWeaponType::Unarmed, { 1, 1, bIgnoreExtraCheck } },
        { EWeaponType::Melee, { .7f, 1, bIgnoreExtraCheck } },
        { EWeaponType::Pistol, { .5f, .8f, bApplyExtraCheck } },
        { EWeaponType::Rifle, { .2f, .8f, bApplyExtraCheck } },
    };

    TemperamentProbabilities =
    {
        { ETemperamentType::Agressive, .7f },
        { ETemperamentType::Moderate, .4f },
        { ETemperamentType::Retreat, .4f },
    };

    TemperamentDistributions =
    {
        { ETemperamentType::Agressive, .6f },
        { ETemperamentType::Moderate, .3f },
        { ETemperamentType::Retreat, .1f },
    };
}

void UEnemyEquipSystem::ProvideWithAvailableWeapons_Implementation(AShooterCharacter* ShooterCharacter)
{
    if (!IsValid(ShooterCharacter)) return;
    const ERanksProgressionSource ProgressionSource = ShooterCharacter->GetRanksProgressionSource();
    UShooterSkillsSystem* SkillsSystem = ShooterCharacter->GetSkillsSystem();
    if (!IsValid(SkillsSystem) || ProgressionSource != ERanksProgressionSource::ProvidedBySystem) return;

    APlayerCharacter* Player = GetPlayerCharacter();
    if (!IsValid(Player)) return;
    UShooterSkillsSystem* PlayerSkills = Player->GetSkillsSystem();
    if (!IsValid(PlayerSkills)) return;

    SkillsSystem->AvailableSlots.Empty();
    TSet<EWeaponType> SlotsToAccess;
    for (const auto &[Type, Chances] : WeaponEquipProbabilities)
    {
        bool bHasWeapon = PlayerSkills->AvailableSlots.Contains(Type);
        if (Chances.bApplyExtraCheck)
        {
            bool bSlotUnlockAllowed = CheckSlotAvailability(PlayerSkills, Type);
            if (!bSlotUnlockAllowed) continue;
        }
        const float Probability = bHasWeapon ? Chances.AlternativeProbability : Chances.DefaultProbability;
        if (Probability < FMath::FRand()) continue;
        SkillsSystem->AvailableSlots.Add(Type);
    }
}

void UEnemyEquipSystem::ProvideWithRanksProgression_Implementation(AShooterCharacter* ShooterCharacter)
{
    if (!IsValid(ShooterCharacter)) return;
    UShooterSkillsSystem* SkillsSystem = ShooterCharacter->GetSkillsSystem();
    const ERanksProgressionSource ProgressionSource = ShooterCharacter->GetRanksProgressionSource();
    if (!IsValid(SkillsSystem) || ProgressionSource != ERanksProgressionSource::ProvidedBySystem) return;

    if (!SkillsSystem->IsProgressionAssetValid()) return;
    UShooterSkillsProgression* Progression = SkillsSystem->GetProgressionAsset();

    SkillsSystem->InitializeDefaultRanks();
    FRanksProgression& EnemyRanks = SkillsSystem->RanksProgression;

    APlayerCharacter* Player = GetPlayerCharacter();
    if (!IsValid(Player)) return;
    UShooterSkillsSystem* PlayerSkills = Player->GetSkillsSystem();
    if (!IsValid(PlayerSkills)) return;
    const FRanksProgression& PlayerRanks = PlayerSkills->RanksProgression;

    const TArray<int8> SkillDeviation = { -1, 0, 1 };
    auto AddDev = [&] (int32 Rank) { return Rank + SkillDeviation[FMath::RandRange(0, SkillDeviation.Num() - 1)]; };
    auto GenerateRank = [&] (int32 Rank, int32 Num) { return FMath::Clamp(AddDev(Rank), 0, Num); };

    EnemyRanks.WeaponReload = GenerateRank(PlayerRanks.WeaponReload, Progression->WeaponReloadSpeed.Num());
    EnemyRanks.WeaponSwap = GenerateRank(PlayerRanks.WeaponSwap, Progression->WeaponSwappingSpeed.Num());
    EnemyRanks.Aiming = GenerateRank(PlayerRanks.Aiming, Progression->AimingSpeed.Num());
    EnemyRanks.Health = GenerateRank(PlayerRanks.Health, Progression->MaxHealth.Num());

    for (const auto &[Key, _] : EnemyRanks.WeaponAttacks)
    {
        const int32 Num = Progression->WeaponAttacks.Num();
        const int32* PlayerRankPtr = PlayerRanks.WeaponAttacks.Find(Key);
        int32 Rank = PlayerRankPtr ? GenerateRank(*PlayerRankPtr, Num) : FMath::RandRange(0, Num - 1);
        EnemyRanks.WeaponAttacks[Key] = Rank;
    }

    for (const auto &[Key, _] : EnemyRanks.AmmoCapaticy)
    {
        const int32 Num = Progression->AmmoStats.Num();
        const int32* PlayerRankPtr = PlayerRanks.AmmoCapaticy.Find(Key);
        int32 Rank = PlayerRankPtr ? GenerateRank(*PlayerRankPtr, Num) : FMath::RandRange(0, Num - 1);
        EnemyRanks.AmmoCapaticy[Key] = Rank;
    }

    for (const auto &[Key, _] : EnemyRanks.WeaponRecoilModifiers)
    {
        const int32 Num = Progression->WeaponRecoilModifiers.Num();
        const int32* PlayerRankPtr = PlayerRanks.WeaponRecoilModifiers.Find(Key);
        int32 Rank = PlayerRankPtr ? GenerateRank(*PlayerRankPtr, Num) : FMath::RandRange(0, Num - 1);
        EnemyRanks.WeaponRecoilModifiers[Key] = Rank;
    }
}

void UEnemyEquipSystem::AdjustBehavior_Implementation(AShooterCharacter* ShooterCharacter)
{
    if (!IsValid(ShooterCharacter)) return;
    auto AIController = ShooterCharacter->GetController<AShooterAIController>();
    if (!IsValid(AIController)) return;

    for (const auto &[Type, Probability] : TemperamentProbabilities)
    {
        if (Probability < FMath::FRand()) continue;
        AIController->Execute_ChangeTemperamentType(AIController, Type);
        break;
    }
}

void UEnemyEquipSystem::DistributeBehaviors_Implementation(const TArray<AShooterCharacter*>& ShooterCharacters)
{
    if (ShooterCharacters.IsEmpty()) return;

    const uint32 EnemiesCount = ShooterCharacters.Num();
    uint32 CurrentEnemy = 0;
    uint32 Distributed = 0;

    for (const auto &[Type, Probability] : TemperamentProbabilities)
    {
        while (CurrentEnemy < EnemiesCount)
        {
            bool bDistributionBoundReached = (Distributed / EnemiesCount) > Probability;
            if (bDistributionBoundReached)
            {
                Distributed = 0;
                break;
            }

            AShooterCharacter* Enemy = ShooterCharacters[CurrentEnemy];
            bool bCanSetTemperament = IsValid(Enemy) && (Enemy->GetRanksProgressionSource() == ERanksProgressionSource::ProvidedBySystem);
            if (bCanSetTemperament)
            {
                auto AIController = Enemy->GetController<AShooterAIController>();
                if (IsValid(AIController)) AIController->Execute_ChangeTemperamentType(AIController, Type);
            }

            ++CurrentEnemy;
            ++Distributed;
        }
    }
}

APlayerCharacter* UEnemyEquipSystem::GetPlayerCharacter() const
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    return IsValid(PlayerController) ? Cast<APlayerCharacter>(PlayerController->GetCharacter()) : nullptr;
}

bool UEnemyEquipSystem::CheckSlotAvailability(UShooterSkillsSystem* SkillsSystem, EWeaponType Slot) const
{
    if (!IsValid(SkillsSystem) || Slot == EWeaponType::None) return true;

    if (Slot == EWeaponType::Rifle)
    {
        const EWeaponType FirearmsSlots[] = { EWeaponType::Pistol, EWeaponType::Rifle };
        for (const auto& ToCheck : FirearmsSlots)
        {
            bool bSlotAvailable = SkillsSystem->AvailableSlots.Contains(Slot);
            if (bSlotAvailable) return true;
        }
        return false;
    }
    else if (Slot == EWeaponType::Pistol)
    {
        return !SkillsSystem->AvailableSlots.Contains(EWeaponType::Unarmed);
    }

    return true;
}
