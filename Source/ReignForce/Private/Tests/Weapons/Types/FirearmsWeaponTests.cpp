#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "Tests/Weapons/Types/FirearmsWeaponTests.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

#include "CoreMinimal.h"
#include "Weapons/Types/FirearmsWeapon.h"
#include "Tests/TestUtils.h"


using namespace TestUtils;

namespace
{
    constexpr const char* TEST_MAP_PATH = "/Game/Core/Maps/Test/TestMap.TestMap'";
    constexpr const char* WEAPON_TO_TEST_PATH = "/Script/Engine.Blueprint'/Game/Core/Weapons/BP_Pistol.BP_Pistol'";
}

BEGIN_DEFINE_SPEC(FFirearmsWeaponTests, "ReignForce.FirearmsWeapon",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

private:
    AFirearmsWeapon* GetSpawnedWeaponInWorld()
    {
        bool IsWorldLoaded = AutomationOpenMap(TEST_MAP_PATH);
        TestTrueExpr(IsWorldLoaded);

        UWorld* World = GetGameWorld();
        TestNotNull("Current test world created", World);

        const UBlueprint* FirearmsWeaponBlueprint = LoadObject<UBlueprint>(nullptr, *FString(WEAPON_TO_TEST_PATH));
        TestNotNull("Firearms blueprint class loaded", FirearmsWeaponBlueprint);

        TSubclassOf<AFirearmsWeapon> FirearmsClass = Cast<UClass, AFirearmsWeapon>(FirearmsWeaponBlueprint->GeneratedClass.Get());
        TestNotNull("Firearms class loaded", FirearmsClass.Get());

        auto FirearmsWeapon = World->SpawnActor<AFirearmsWeapon>(FirearmsClass);
        TestNotNull("Firearms weapon instance created", FirearmsWeapon);

        return FirearmsWeapon;
    }

END_DEFINE_SPEC(FFirearmsWeaponTests);

void FFirearmsWeaponTests::Define()
{
    It("AmmoCheck", [this]
    {
        AFirearmsWeapon* FirearmsWeapon = GetSpawnedWeaponInWorld();

        TestTrue("Firearms ammo should be empty by default", FirearmsWeapon->GetIsAmmoEmpty());
        TestFalse("Firearms ammo should be not full by default", FirearmsWeapon->GetIsAmmoFull());

        FirearmsWeapon->RefillAmmo(FirearmsWeapon->GetMaxAmmo());

        TestFalse("Firearms ammo should be empty after refill", FirearmsWeapon->GetIsAmmoEmpty());
        TestTrue("Firearms ammo should be full after refill", FirearmsWeapon->GetIsAmmoFull());
    });

    It("PerformsShot", [this]
    {
        AFirearmsWeapon* FirearmsWeapon = GetSpawnedWeaponInWorld();
        FirearmsWeapon->RefillAmmo(FirearmsWeapon->GetMaxAmmo());

        const int32 AmmoBeforeShot = FirearmsWeapon->GetAmmoCount();
        const int32 AmmoShouldLeft = AmmoBeforeShot - 1;

        bool bShotPerformed = FirearmsWeapon->Fire();
        TestTrueExpr(bShotPerformed);

        const int32 AmmoAfterShot = FirearmsWeapon->GetAmmoCount();
        TestEqual("Ammo should be decreased by one after a single shot", AmmoAfterShot, AmmoShouldLeft);
    });
};

#endif
