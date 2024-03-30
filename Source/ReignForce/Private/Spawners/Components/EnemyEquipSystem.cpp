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
#include "Weapons/WeaponType.h"


namespace
{
    void GenerateDeviatedRankForWeaponTypeCollection(TMap<EWeaponType, int32>& Collection, int32 RanksCount, const TMap<EWeaponType, int32>& PlayerRanks, const FRankDeviation& Deviation = {})
    {
        for (const auto &[Key, _] : Collection)
        {
            const int32* PlayerRankPtr = PlayerRanks.Find(Key);
            const int32 DeviatedRank = PlayerRankPtr ? Deviation.Apply(*PlayerRankPtr, 0, RanksCount) : FMath::RandRange(0, RanksCount - 1);
            Collection[Key] = DeviatedRank;
        }
    }

    void SetRankValueToAllWeaponTypeCollection(TMap<EWeaponType, int32>& Collection, int32 Rank)
    {
        for (const auto &[Key, _] : Collection) Collection[Key] = Rank;
    }
}

int32 FRankDeviation::Apply(int32 Rank, int32 LowerBound, int32 UpperBound) const
{
    return bApplyDeviation ? FMath::Clamp(FMath::RandRange(Rank - Left, Rank + Right), LowerBound, UpperBound) : Rank;
}

int32 FRankDeviation::operator()(int32 Rank, int32 LowerBound, int32 UpperBound) const
{
    return Apply(Rank, LowerBound, UpperBound);
}

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

    MaxHealthDeviation = FRankDeviation(2, 0);
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

    EnemyRanks.WeaponReload = WeaponReloadSpeedDeviation(PlayerRanks.WeaponReload, 0, Progression->WeaponReloadSpeed.Num());
    EnemyRanks.WeaponSwap = WeaponSwapSpeedDeviation(PlayerRanks.WeaponSwap, 0, Progression->WeaponSwappingSpeed.Num());
    EnemyRanks.Aiming = AimingSpeedDeviation(PlayerRanks.Aiming, 0, Progression->AimingSpeed.Num());
    EnemyRanks.Health = MaxHealthDeviation(PlayerRanks.Health, 0, Progression->MaxHealth.Num());

    GenerateDeviatedRankForWeaponTypeCollection(EnemyRanks.WeaponAttacks, Progression->WeaponAttacks.Num(), PlayerRanks.WeaponAttacks, DefaultDeviation);
    GenerateDeviatedRankForWeaponTypeCollection(EnemyRanks.WeaponRecoilModifiers, Progression->WeaponRecoilModifiers.Num(), PlayerRanks.WeaponRecoilModifiers, DefaultDeviation);

    constexpr uint8 DefaultAmmoCapacityRank = 0;
    SetRankValueToAllWeaponTypeCollection(EnemyRanks.AmmoCapacity, DefaultAmmoCapacityRank);
    // GenerateDeviatedRankForWeaponTypeCollection(EnemyRanks.AmmoCapacity, Progression->AmmoStats.Num(), PlayerRanks.AmmoCapacity, DefaultDeviation);
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

    if (UWeaponSlotsSystem::IsFirearmsType(Slot))
    {
        const EWeaponType FirearmsSlots[] = { EWeaponType::Pistol, EWeaponType::Rifle };
        for (const auto& ToCheck : FirearmsSlots)
        {
            bool bSlotAvailable = SkillsSystem->AvailableSlots.Contains(Slot);
            if (bSlotAvailable) return true;
        }
        return false;
    }

    return true;
}
