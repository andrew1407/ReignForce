#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "Tests/TestUtils.h"
#include "GameStates/Components/UpgradesProgressStateComponent.h"
#include "GameStates/Components/RoundDifficultyProgression.h"
#include "ShooterCharacter/Stats/ShooterSkillsSystem.h"
#include "ShooterCharacter/Stats/ShooterSkillsProgression.h"


using namespace TestUtils;

namespace
{
    FDifficultyValueContainer MakeValue(int32 Value)
    {
        FDifficultyValueContainer Container;
        Container.Value = Value;
        return Container;
    }

    FDifficultyValueContainer MakeRange(int32 Min, int32 Max)
    {
        FDifficultyValueContainer Container;
        Container.bValueRange = true;
        Container.ValueMin = Min;
        Container.ValueMax = Max;
        return Container;
    }
}

BEGIN_DEFINE_SPEC(FProgressionTests, "ReignForce.Unit.Progression",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);
END_DEFINE_SPEC(FProgressionTests);

void FProgressionTests::Define()
{
    Describe("UpgradesProgressState", [this]
    {
        It("ClampsSkillsToAllowedRange", [this]
        {
            auto Progress = NewObject<UUpgradesProgressStateComponent>(GetTransientPackage());
            const int32 MaxSkills = Progress->GetMaxSkillsAllowed();

            TestFalse("Negative skills rejected", Progress->SetSkillsCount(-1));
            TestTrue("Skills set", Progress->SetSkillsCount(MaxSkills + 10));
            TestEqual("Skills clamped to the max allowed", Progress->GetSkillsCount(), MaxSkills);
            TestTrue("Full of skills", Progress->IsFullOfSkills());
            TestFalse("No skills added when full", Progress->AddSkills(1));
        });

        It("AddsSkillsUpToTheGainedLimit", [this]
        {
            auto Progress = NewObject<UUpgradesProgressStateComponent>(GetTransientPackage());
            const int32 MaxSkills = Progress->GetMaxSkillsAllowed();

            Progress->SetSkillsCount(5);
            Progress->SetSkillsSpentCount(10);
            TestFalse("Zero skills aren't added", Progress->AddSkills(0));
            TestTrue("Skills added", Progress->AddSkills(1000));
            TestEqual("Available + spent never exceed the max", Progress->GetSkillsCount() + Progress->GetSkillsSpentCount(), MaxSkills);
            TestEqual("Gained skills balance", Progress->GetProgressionState().GetSkillsGained(), MaxSkills);
        });

        It("TakesAndDropsSkills", [this]
        {
            auto Progress = NewObject<UUpgradesProgressStateComponent>(GetTransientPackage());
            Progress->SetSkillsCount(4);

            TestFalse("Nothing to take", Progress->CanTake(0));
            TestFalse("Too many to take", Progress->TryTakeSkills(5));
            TestTrue("Skills taken", Progress->TryTakeSkills(3));
            TestEqual("Skills left", Progress->GetSkillsCount(), 1);

            TestTrue("Spent skills accumulated", Progress->AccumulateSpentSkills(3));
            TestFalse("Non positive spent skills rejected", Progress->AccumulateSpentSkills(0));
            TestEqual("Spent skills", Progress->GetSkillsSpentCount(), 3);

            TestTrue("Skills dropped", Progress->DropSkills());
            TestEqual("No skills after drop", Progress->GetSkillsCount(), 0);
            TestFalse("Nothing to drop", Progress->DropSkills());
        });

        It("CountsUnlockedUpgrades", [this]
        {
            auto Progress = NewObject<UUpgradesProgressStateComponent>(GetTransientPackage());
            const int32 MaxUpgrades = Progress->GetMaxUpgrades();

            TestFalse("Negative upgrades rejected", Progress->SetUpgradesCount(-2));
            TestTrue("Upgrades set", Progress->SetUpgradesCount(3));
            TestTrue("Upgrades added", Progress->AddUnlokedUpgrades(2));
            TestEqual("Upgrades unlocked", Progress->GetUpgradesUnlocked(), 5);
            TestFalse("Negative added upgrades rejected", Progress->AddUnlokedUpgrades(-1));

            Progress->AddUnlokedUpgrades(MaxUpgrades * 2);
            TestEqual("Upgrades clamped", Progress->GetUpgradesUnlocked(), MaxUpgrades);
            TestTrue("Fully upgraded", Progress->IsFullyUpgraded());
            TestFalse("No upgrades over the max", Progress->AddUnlokedUpgrades());
        });
    });

    Describe("RoundDifficulty", [this]
    {
        It("ComparesValuesByMode", [this]
        {
            const FDifficultyValueContainer Value = MakeValue(4);
            TestTrue("Exact value fits", Value.FitsByComparingValue(4));
            TestFalse("Other value doesn't fit", Value.FitsByComparingValue(5));
            TestEqual("Exact value", Value.GetValueByMode(), 4);

            const FDifficultyValueContainer Range = MakeRange(2, 6);
            TestTrue("Lower bound fits", Range.FitsByComparingValue(2));
            TestTrue("Upper bound fits", Range.FitsByComparingValue(6));
            TestFalse("Out of range", Range.FitsByComparingValue(7));
            for (int32 I = 0; I < 20; ++I)
            {
                const int32 Random = Range.GetValueByMode();
                if (!TestTrue(FString::Printf(TEXT("Random value %d in range"), Random), Random >= 2 && Random <= 6)) break;
            }
        });

        It("PicksEnemiesCountByGainedSkills", [this]
        {
            auto Difficulty = NewObject<URoundDifficultyProgression>(GetTransientPackage());
            TestEqual("Default round reward", Difficulty->GetSkillsRewardForRoundWin(), 3);
            TestEqual("No options - no enemies", Difficulty->GetEnemiesCountBySkills(1), 0);

            FDifficultyOptions Beginner;
            Beginner.NecessarySkills = MakeRange(0, 5);
            Beginner.EnemiesToSpawn = MakeValue(2);
            FDifficultyOptions Advanced;
            Advanced.NecessarySkills = MakeValue(6);
            Advanced.EnemiesToSpawn = MakeRange(3, 4);
            const bool bSet = SetPropertyValue(Difficulty, TEXT("EnemiesPerRoundByGainedSkills"), TArray<FDifficultyOptions>{ Beginner, Advanced });
            if (!TestTrue("Difficulty options set", bSet)) return;

            TestEqual("Beginner enemies", Difficulty->GetEnemiesCountBySkills(3), 2);
            const int32 AdvancedEnemies = Difficulty->GetEnemiesCountBySkills(6);
            TestTrue("Advanced enemies in range", AdvancedEnemies >= 3 && AdvancedEnemies <= 4);
            TestEqual("Unmatched skills - no enemies", Difficulty->GetEnemiesCountBySkills(7), 0);
        });
    });

    Describe("SkillsSystem", [this]
    {
        It("FallsBackToDefaultsWithoutProgression", [this]
        {
            auto Skills = NewObject<UShooterSkillsSystem>(GetTransientPackage());
            TestFalse("No progression asset", Skills->IsProgressionAssetValid());
            TestTrue("Unarmed slot available by default", Skills->AvailableSlots.Contains(EWeaponType::Unarmed));

            TestEqual("Aiming rate default", Skills->GetAimingRateOrDefault(1.5f), 1.5f);
            TestEqual("Swapping rate default", Skills->GetSwappingRateOrDefault(), 1.f);
            TestEqual("Reload rate default", Skills->GetWeaponReloadRateOrDefault(), 1.f);
            TestEqual("Health default", Skills->GetHealthOrDefault(77), 77);
            TestEqual("Ammo default", Skills->GetAmmoCapacityOrDefault(EWeaponType::Pistol, 9), 9);
            TestTrue("Recoil modifier default", Skills->GetRecoilModifierOrDefault(EWeaponType::Rifle).Equals(FVector2D(1, 1)));
            TestEqual("Hit chance default", Skills->GetAttackHitChanceOrDefault(EWeaponType::Melee), 1.f);
            TestTrue("Spread default", Skills->GetFirearmsProjectileSpreadOrDefault(EWeaponType::Rifle).IsZero());
            TestTrue("No aim by default", Skills->GetAimAvailabilityOrDefault() == EAimAvailability::NoAim);
            TestTrue("Continuous attack default", Skills->GetContituousAttackOrDefault(EWeaponType::Rifle, true));
        });

        It("ResolvesStatsByRanks", [this]
        {
            auto Progression = NewObject<UShooterSkillsProgression>(GetTransientPackage());
            Progression->AimingSpeed = { 1, 1.5f, 2 };
            Progression->MaxHealth = { 100, 150 };
            Progression->AimAvailability = { EAimAvailability::NoAim, EAimAvailability::AimPresent };
            Progression->FirearmsAutoReload = ESkillUnlockAbility::AlwaysTrue;
            Progression->AmmoSaveMode = ESkillUnlockAbility::FromFalseToTrue;
            Progression->ContinuousAttacks = { { EWeaponType::Rifle, ESkillUnlockAbility::FromTrueToFalse }, { EWeaponType::Pistol, ESkillUnlockAbility::FromFalseToTrue } };
            Progression->AmmoStats.Add(EWeaponType::Pistol).MaxCount = { 12, 18 };
            Progression->WeaponRecoilModifiers.Add(EWeaponType::Rifle).Collection = { FVector2D(1, 1), FVector2D(.5f, .25f) };

            auto Skills = NewObject<UShooterSkillsSystem>(GetTransientPackage());
            if (!TestTrue("Progression asset set", SetPropertyValue(Skills, TEXT("ProgressionAsset"), TObjectPtr<UShooterSkillsProgression>(Progression)))) return;
            TestTrue("Progression asset valid", Skills->IsProgressionAssetValid());

            Skills->InitializeDefaultRanks();
            const FRanksProgression& Ranks = Skills->RanksProgression;
            TestTrue("Always true auto reload enabled", Ranks.bWeaponAutoReloadMode);
            TestFalse("Locked ammo save mode disabled", Ranks.bSaveAmmoMode);
            TestTrue("Continuous rifle attack enabled initially", Skills->GetContituousAttackOrDefault(EWeaponType::Rifle));
            TestFalse("Continuous pistol attack locked initially", Skills->GetContituousAttackOrDefault(EWeaponType::Pistol));
            TestFalse("Unknown weapon has no continuous attack", Skills->GetContituousAttackOrDefault(EWeaponType::Melee, true));
            TestTrue("Ammo rank initialized", Ranks.AmmoCapacity.Contains(EWeaponType::Pistol));

            TestEqual("Initial pistol ammo", Skills->GetAmmoCapacityOrDefault(EWeaponType::Pistol), 12);
            Skills->RanksProgression.AmmoCapacity[EWeaponType::Pistol] = 1;
            TestEqual("Upgraded pistol ammo", Skills->GetAmmoCapacityOrDefault(EWeaponType::Pistol), 18);
            TestEqual("Rifle ammo not in progression", Skills->GetAmmoCapacityOrDefault(EWeaponType::Rifle, 5), 5);

            Skills->RanksProgression.Aiming = 2;
            TestEqual("Top aiming rank", Skills->GetAimingRateOrDefault(), 2.f);
            Skills->RanksProgression.Aiming = 10;
            TestEqual("Rank out of stages falls back", Skills->GetAimingRateOrDefault(.3f), .3f);

            Skills->RanksProgression.Health = 1;
            TestEqual("Upgraded health", Skills->GetHealthOrDefault(), 150);

            Skills->RanksProgression.WeaponRecoilModifiers[EWeaponType::Rifle] = 1;
            TestTrue("Upgraded recoil", Skills->GetRecoilModifierOrDefault(EWeaponType::Rifle).Equals(FVector2D(.5f, .25f)));

            Skills->RanksProgression.AimAvailability = 1;
            TestTrue("Aim unlocked", Skills->GetAimAvailabilityOrDefault() == EAimAvailability::AimPresent);
        });
    });
}

#endif
