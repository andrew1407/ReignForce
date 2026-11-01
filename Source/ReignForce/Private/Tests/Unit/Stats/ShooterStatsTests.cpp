#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "ShooterCharacter/Stats/ShooterStatsComponent.h"
#include "ShooterCharacter/Stats/PointsAccumulator.h"
#include "Weapons/WeaponType.h"


namespace
{
    constexpr float MAX_HEALTH = 100;
    constexpr int32 PISTOL_AMMO = 12;
}

BEGIN_DEFINE_SPEC(FShooterStatsTests, "ReignForce.Unit.ShooterStats",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

    UShooterStatsComponent* Stats = nullptr;

END_DEFINE_SPEC(FShooterStatsTests);

void FShooterStatsTests::Define()
{
    BeforeEach([this]
    {
        Stats = NewObject<UShooterStatsComponent>(GetTransientPackage());
    });

    AfterEach([this]
    {
        Stats = nullptr;
    });

    Describe("Health", [this]
    {
        It("StartsWithoutHealth", [this]
        {
            TestTrue("No health by default", Stats->HasNoHealth());
            TestFalse("Regenerator disabled by default", Stats->bHealthRegeneratorEnabled);
            TestFalse("Dead shooter can't take damage", Stats->TakeDamage(10));
        });

        It("SetsMaxHealthWithRefill", [this]
        {
            Stats->SetMaxHealth(MAX_HEALTH);
            TestEqual("Max health set", Stats->GetMaxHealth(), MAX_HEALTH);
            TestEqual("Current health kept", Stats->GetCurrentHealth(), 0.f);

            constexpr bool bRefill = true;
            Stats->SetMaxHealth(MAX_HEALTH, bRefill);
            TestTrue("Refilled to max", Stats->HasMaxHealth());
            TestEqual("Full health percent", Stats->GetCurrentHealthPercent(), 1.f);
        });

        It("ClampsDamageAndHealing", [this]
        {
            Stats->SetMaxHealth(MAX_HEALTH, true);

            TestFalse("Healing at max health is skipped", Stats->AddHealthPoints(10));

            TestTrue("Damage taken", Stats->TakeDamage(30));
            TestEqual("Health after damage", Stats->GetCurrentHealth(), 70.f);
            TestEqual("Health percent after damage", Stats->GetCurrentHealthPercent(), .7f, 1e-4f);

            TestTrue("Healed", Stats->AddHealthPoints(500));
            TestEqual("Healing clamped to max", Stats->GetCurrentHealth(), MAX_HEALTH);

            TestTrue("Lethal damage taken", Stats->SubstractHealthPoints(1000));
            TestEqual("Health clamped to zero", Stats->GetCurrentHealth(), 0.f);
            TestTrue("No health left", Stats->HasNoHealth());
            TestFalse("No more damage when dead", Stats->TakeDamage(1));

            Stats->RefillHealthToMax();
            TestTrue("Refill restores max health", Stats->HasMaxHealth());
        });
    });

    Describe("Ammo", [this]
    {
        It("TreatsMissingWeaponAsEmpty", [this]
        {
            TestEqual("No ammo", Stats->GetAmmoCount(EWeaponType::Rifle), 0);
            TestEqual("No max ammo", Stats->GetMaxAmmoCount(EWeaponType::Rifle), 0);
            TestTrue("Empty", Stats->IsAmmoEmpty(EWeaponType::Rifle));
            TestFalse("Not full", Stats->IsAmmoFull(EWeaponType::Rifle));
            TestFalse("Can't add ammo", Stats->AddAmmoTo(EWeaponType::Rifle, 5));
            TestFalse("Can't set ammo", Stats->SetWeaponAmmo(EWeaponType::Rifle, 5));
            TestEqual("Nothing to take", Stats->GetAmmoAsRest(EWeaponType::Rifle, 5), 0);
            TestFalse("Nothing to remove", Stats->RemoveWeaponAmmo(EWeaponType::Rifle));
        });

        It("AddsWeaponAmmoOnce", [this]
        {
            TestTrue("Full pistol ammo added", Stats->AddWeaponAmmo(EWeaponType::Pistol, PISTOL_AMMO));
            TestTrue("Pistol is full", Stats->IsAmmoFull(EWeaponType::Pistol));
            TestFalse("Weapon ammo is added only once", Stats->AddWeaponAmmo(EWeaponType::Pistol, 99));
            TestEqual("Max ammo kept", Stats->GetMaxAmmoCount(EWeaponType::Pistol), PISTOL_AMMO);

            constexpr bool bFull = false;
            TestTrue("Empty rifle ammo added", Stats->AddWeaponAmmo(EWeaponType::Rifle, 30, bFull));
            TestTrue("Rifle is empty", Stats->IsAmmoEmpty(EWeaponType::Rifle));
            TestEqual("Rifle max ammo", Stats->GetMaxAmmoCount(EWeaponType::Rifle), 30);
        });

        It("TakesAmmoAsRest", [this]
        {
            Stats->AddWeaponAmmo(EWeaponType::Pistol, PISTOL_AMMO);

            TestEqual("Enough ammo taken fully", Stats->GetAmmoAsRest(EWeaponType::Pistol, 5), 5);
            TestEqual("Ammo left", Stats->GetAmmoCount(EWeaponType::Pistol), PISTOL_AMMO - 5);

            TestEqual("Only the rest is taken", Stats->GetAmmoAsRest(EWeaponType::Pistol, 100), PISTOL_AMMO - 5);
            TestTrue("Ammo spent", Stats->IsAmmoEmpty(EWeaponType::Pistol));
            TestEqual("Nothing to take when empty", Stats->GetAmmoAsRest(EWeaponType::Pistol, 1), 0);
        });

        It("ClampsAmmoToCapacity", [this]
        {
            Stats->AddWeaponAmmo(EWeaponType::Pistol, PISTOL_AMMO, false);

            TestTrue("Ammo added", Stats->AddAmmoTo(EWeaponType::Pistol, 100));
            TestEqual("Added ammo clamped", Stats->GetAmmoCount(EWeaponType::Pistol), PISTOL_AMMO);
            TestFalse("Full weapon takes no ammo", Stats->AddAmmoTo(EWeaponType::Pistol, 1));

            TestTrue("Ammo set", Stats->SetWeaponAmmo(EWeaponType::Pistol, -5));
            TestEqual("Negative ammo clamped", Stats->GetAmmoCount(EWeaponType::Pistol), 0);

            TestTrue("Capacity raised", Stats->SetWeaponMaxAmmo(EWeaponType::Pistol, 20));
            Stats->SetWeaponAmmo(EWeaponType::Pistol, 18);
            TestEqual("Ammo fits the raised capacity", Stats->GetAmmoCount(EWeaponType::Pistol), 18);

            TestTrue("Weapon ammo removed", Stats->RemoveWeaponAmmo(EWeaponType::Pistol));
            TestEqual("Removed weapon has no capacity", Stats->GetMaxAmmoCount(EWeaponType::Pistol), 0);
        });
    });

    Describe("WeaponModels", [this]
    {
        It("ReturnsDefaultForMissingModel", [this]
        {
            TestEqual("Default model", Stats->GetWeaponModelOrDefault(EWeaponType::Rifle, 3), 3);
        });

        It("OverwritesModel", [this]
        {
            Stats->SetWeaponModelOf(EWeaponType::Rifle, 1);
            TestEqual("Model set", Stats->GetWeaponModelOrDefault(EWeaponType::Rifle), 1);
            Stats->SetWeaponModelOf(EWeaponType::Rifle, 2);
            TestEqual("Model changed", Stats->GetWeaponModelOrDefault(EWeaponType::Rifle), 2);
        });
    });

    Describe("PointsAccumulator", [this]
    {
        It("AccumulatesBySpeedUntilThreshold", [this]
        {
            FPointsAccumulator Accumulator;
            Accumulator.Speed = 2;
            Accumulator.Threshold = 5;

            Accumulator.AccumulateCurrent(2);
            TestEqual("Speed applied", Accumulator.CurrentValue, 4.f);
            TestFalse("Threshold not reached", Accumulator.ThresholdReached());

            Accumulator.AccumulateCurrent(1, 1);
            TestTrue("Threshold reached with custom speed", Accumulator.ThresholdReached());

            TestEqual("Accumulated dropped", Accumulator.DropAccumulated(), 5.f);
            TestEqual("Reset after drop", Accumulator.CurrentValue, 0.f);
        });
    });
}

#endif
