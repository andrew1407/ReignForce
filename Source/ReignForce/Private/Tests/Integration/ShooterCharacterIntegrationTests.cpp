#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "UObject/StrongObjectPtr.h"

#include "Tests/TestUtils.h"
#include "Enemy/EnemyCharacter.h"
#include "ReinforcementLearning/AICommandsUtils.h"
#include "ShooterCharacter/Stats/ShooterStatsComponent.h"
#include "Utils/Cooldown.h"
#include "Weapons/Components/WeaponSlotsSystem.h"
#include "Weapons/Components/CombatSystem.h"
#include "Weapons/Types/FirearmsWeapon.h"
#include "Weapons/ShooterState.h"


using namespace TestUtils;

namespace
{
    constexpr double SPAWN_SETTLE_TIMEOUT = 5;
    const FVector SPAWN_OFFSET(600, 0, 100);
}

BEGIN_DEFINE_SPEC(FShooterCharacterIntegrationTests, "ReignForce.Integration.ShooterCharacter",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

    TWeakObjectPtr<AEnemyCharacter> Enemy;
    TStrongObjectPtr<UCooldown> Cooldown;

    FVector GetSpawnLocation(UWorld* World) const
    {
        AShooterCharacter* Player = EnsurePlayerShooter(World);
        return (IsValid(Player) ? Player->GetActorLocation() : FVector::ZeroVector) + SPAWN_OFFSET;
    }

    /** Spawns the enemy blueprint and waits for it to be on the ground with its stats defined */
    void SpawnEnemy(const FDoneDelegate& Done)
    {
        UWorld* World = GetGameWorld();
        Enemy = Cast<AEnemyCharacter>(SpawnBlueprintActor(World, ENEMY_BLUEPRINT_PATH, GetSpawnLocation(World)));
        if (!TestTrue("Enemy spawned", Enemy.IsValid())) return Done.Execute();

        WaitUntil(Done, [WeakEnemy = Enemy]
        {
            if (!WeakEnemy.IsValid()) return true;
            const bool bOnGround = !WeakEnemy->GetCharacterMovement()->IsFalling();
            return bOnGround && WeakEnemy->GetHealthPoints() > 0;
        }, SPAWN_SETTLE_TIMEOUT);
    }

END_DEFINE_SPEC(FShooterCharacterIntegrationTests);

void FShooterCharacterIntegrationTests::Define()
{
    LatentBeforeEach([this](const FDoneDelegate& Done) { OpenTestMap(*this, Done); });

    AfterEach([this]
    {
        if (Enemy.IsValid()) Enemy->Destroy();
        Enemy.Reset();
        Cooldown.Reset();
    });

    Describe("Cooldown", [this]
    {
        LatentIt("ExpiresAfterItsDuration", [this](const FDoneDelegate& Done)
        {
            UWorld* World = GetGameWorld();
            if (!TestNotNull("World", World)) return Done.Execute();
            Cooldown.Reset(NewObject<UCooldown>(World));
            UObject* Target = World->GetWorldSettings();

            TestFalse("No cooldown initially", Cooldown->HasCooldown(Target));
            Cooldown->Activate(Target, .2f);
            TestTrue("Cooldown active", Cooldown->HasCooldown(Target));
            TestFalse("Refresh of an inactive target fails", Cooldown->Refresh(Cooldown.Get(), .2f));

            FDoneDelegate OnExpired = FDoneDelegate::CreateLambda([this, Done, Target]
            {
                TestFalse("Cooldown expired", Cooldown.IsValid() && Cooldown->HasCooldown(Target));
                Done.Execute();
            });
            WaitFor(OnExpired, .5);
        });

        LatentIt("IsProlongedByRefresh", [this](const FDoneDelegate& Done)
        {
            UWorld* World = GetGameWorld();
            if (!TestNotNull("World", World)) return Done.Execute();
            Cooldown.Reset(NewObject<UCooldown>(World));
            UObject* Target = World->GetWorldSettings();

            Cooldown->Activate(Target, .3f);
            FDoneDelegate OnRefreshTime = FDoneDelegate::CreateLambda([this, Done, Target]
            {
                constexpr bool bApplyRefresh = true;
                Cooldown->Activate(Target, 1, bApplyRefresh);
                FDoneDelegate OnFirstDurationPassed = FDoneDelegate::CreateLambda([this, Done, Target]
                {
                    TestTrue("Refreshed cooldown still active past the first duration", Cooldown->HasCooldown(Target));
                    Done.Execute();
                });
                WaitFor(OnFirstDurationPassed, .4);
            });
            WaitFor(OnRefreshTime, .2);
        });
    });

    Describe("Damage", [this]
    {
        LatentBeforeEach([this](const FDoneDelegate& Done) { SpawnEnemy(Done); });

        It("DefinesFullHealthOnSpawn", [this]
        {
            if (!TestTrue("Enemy alive", Enemy.IsValid())) return;
            TestTrue("Shooter components valid", Enemy->IsShooterComponentsValid());
            TestFalse("Not dead", Enemy->IsDead());
            TestEqual("Full health", Enemy->GetHealthPoints(true), 1.f, 1e-3f);
        });

        It("SubstractsAppliedDamage", [this]
        {
            if (!TestTrue("Enemy alive", Enemy.IsValid())) return;
            const float HealthBefore = Enemy->GetHealthPoints();
            const float Damage = HealthBefore / 4;

            UGameplayStatics::ApplyDamage(Enemy.Get(), Damage, nullptr, nullptr, UDamageType::StaticClass());
            TestEqual("Health decreased by the damage", Enemy->GetHealthPoints(), HealthBefore - Damage, 1e-3f);
            TestFalse("Still alive", Enemy->IsDead());
        });

        It("IgnoresDamageWhenInvincible", [this]
        {
            if (!TestTrue("Enemy alive", Enemy.IsValid())) return;
            Enemy->bInvincible = true;
            const float HealthBefore = Enemy->GetHealthPoints();
            UGameplayStatics::ApplyDamage(Enemy.Get(), HealthBefore, nullptr, nullptr, UDamageType::StaticClass());
            TestEqual("Health kept", Enemy->GetHealthPoints(), HealthBefore);
            TestFalse("Not dead", Enemy->IsDead());
        });

        It("PerishesOnLethalDamage", [this]
        {
            if (!TestTrue("Enemy alive", Enemy.IsValid())) return;
            UGameplayStatics::ApplyDamage(Enemy.Get(), Enemy->GetHealthPoints() * 2, nullptr, nullptr, UDamageType::StaticClass());

            TestTrue("No health", Enemy->GetStatsComponent()->HasNoHealth());
            TestTrue("Dead", Enemy->IsDead());
            TestTrue("Dead combat state", Enemy->GetCombatShooterState() == EShooterState::Dead);
            TestTrue("Capsule collision disabled", Enemy->GetCapsuleComponent()->GetCollisionEnabled() == ECollisionEnabled::NoCollision);
            TestTrue("Movement stopped", Enemy->GetCharacterMovement()->MovementMode == MOVE_None);

            UGameplayStatics::ApplyDamage(Enemy.Get(), 10, nullptr, nullptr, UDamageType::StaticClass());
            TestTrue("Still dead after more damage", Enemy->IsDead());
        });

        It("ReportsMetricsForTheModel", [this]
        {
            if (!TestTrue("Enemy alive", Enemy.IsValid())) return;
            FShooterMetrics Metrics;
            UAICommandsUtils::SetMetricsByCharacter(Enemy.Get(), Metrics);

            TestEqual("Raw health", Metrics.Health, Enemy->GetHealthPoints());
            TestEqual("Health percent", Metrics.HealthPercent, Enemy->GetHealthPoints(true));
            TestEqual("Shooter state", Metrics.ShooterState, static_cast<int32>(Enemy->GetCombatShooterState()));
            TestEqual("Active weapon", Metrics.ActiveWeapon, static_cast<int32>(Enemy->GetSelectedWeaponType()));
            TestEqual("Firearms flag", Metrics.IsFirearmsActive, UWeaponSlotsSystem::IsFirearmsType(Enemy->GetSelectedWeaponType()));
        });
    });

    Describe("Weapons", [this]
    {
        LatentBeforeEach([this](const FDoneDelegate& Done) { SpawnEnemy(Done); });

        It("RejectsMismatchedWeaponSlot", [this]
        {
            if (!TestTrue("Enemy alive", Enemy.IsValid())) return;
            UClass* PistolClass = LoadBlueprintClass(PISTOL_BLUEPRINT_PATH);
            if (!TestNotNull("Pistol class", PistolClass)) return;
            TestFalse("Firearms in the melee slot", Enemy->TryAddWeapon(PistolClass, EWeaponType::MELEE));
            TestFalse("No slot", Enemy->TryAddWeapon(PistolClass, EWeaponType::NONE));
        });

        It("DestroysAttachedWeaponsWithTheCharacter", [this]
        {
            if (!TestTrue("Enemy alive", Enemy.IsValid())) return;
            UWeaponSlotsSystem* Slots = Enemy->GetWeaponSlotsSystem();
            UClass* PistolClass = LoadBlueprintClass(PISTOL_BLUEPRINT_PATH);
            UClass* MacheteClass = LoadBlueprintClass(MACHETE_BLUEPRINT_PATH);
            if (!TestNotNull("Pistol class", PistolClass) || !TestNotNull("Machete class", MacheteClass)) return;

            if (Slots->HasFreeSlot(EWeaponType::PISTOL)) TestTrue("Pistol added", Enemy->TryAddWeapon(PistolClass, EWeaponType::PISTOL));
            if (Slots->HasFreeSlot(EWeaponType::MELEE)) TestTrue("Machete added", Enemy->TryAddWeapon(MacheteClass, EWeaponType::MELEE));

            TArray<TWeakObjectPtr<AWeapon>> Weapons;
            for (EWeaponType Slot : { EWeaponType::PISTOL, EWeaponType::MELEE, EWeaponType::RIFLE })
                if (AWeapon* Weapon = Slots->GetWeaponFromSlot(Slot)) Weapons.Add(Weapon);
            TestTrue("Several weapons attached", Weapons.Num() >= 2);

            // Removing them all used to iterate a map while removing from it and leave weapons spawned
            Enemy->Destroy();
            for (const TWeakObjectPtr<AWeapon>& Weapon : Weapons)
                TestFalse(FString::Printf(TEXT("Weapon %s destroyed with its owner"), Weapon.IsValid() ? *Weapon->GetName() : TEXT("-")), IsValid(Weapon.Get()));
        });

        It("RemovesAllWeaponsFromSlots", [this]
        {
            if (!TestTrue("Enemy alive", Enemy.IsValid())) return;
            UWeaponSlotsSystem* Slots = Enemy->GetWeaponSlotsSystem();
            UClass* PistolClass = LoadBlueprintClass(PISTOL_BLUEPRINT_PATH);
            UClass* MacheteClass = LoadBlueprintClass(MACHETE_BLUEPRINT_PATH);
            if (!TestNotNull("Pistol class", PistolClass) || !TestNotNull("Machete class", MacheteClass)) return;
            if (Slots->HasFreeSlot(EWeaponType::PISTOL)) Enemy->TryAddWeapon(PistolClass, EWeaponType::PISTOL);
            if (Slots->HasFreeSlot(EWeaponType::MELEE)) Enemy->TryAddWeapon(MacheteClass, EWeaponType::MELEE);

            Slots->RemoveAllWeapons();
            TestTrue("Pistol slot free", Slots->HasFreeSlot(EWeaponType::PISTOL));
            TestTrue("Melee slot free", Slots->HasFreeSlot(EWeaponType::MELEE));
            TestTrue("Unarmed is always valid", Slots->IsWeaponInSlotValid(EWeaponType::UNARMED));
        });
    });
}

#endif
