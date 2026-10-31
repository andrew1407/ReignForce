#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "Tests/Weapons/Types/FirearmsWeaponTests.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "Weapons/Types/FirearmsWeapon.h"
#include "Tests/TestUtils.h"


using namespace TestUtils;

namespace
{
    constexpr const TCHAR* TEST_MAP_PATH = TEXT("/Game/Core/Maps/Test/TestMap");
    constexpr const TCHAR* TEST_MAP_NAME = TEXT("TestMap");
    constexpr const char* WEAPON_TO_TEST_PATH = "/Script/Engine.Blueprint'/Game/Core/Weapons/BP_Pistol.BP_Pistol'";
    constexpr double WORLD_LOAD_TIMEOUT = 20.0;
}

BEGIN_DEFINE_SPEC(FFirearmsWeaponTests, "ReignForce.FirearmsWeapon",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

private:
    // AutomationOpenMap queues the editor PIE start behind the spec's own latent steps (UE 5.8+),
    // so the test map PIE session is started here directly and awaited before each test
    void OpenTestMap(const FDoneDelegate& Done)
    {
        UWorld* GameWorld = GetGameWorld();
        if (!GameWorld || UWorld::RemovePIEPrefix(GameWorld->GetMapName()) != TEST_MAP_NAME)
        {
            if (GEditor->PlayWorld) GEditor->EndPlayMap();

            const bool bIsWorldLoaded = FEditorFileUtils::LoadMap(TEST_MAP_PATH, false, false);
            TestTrueExpr(bIsWorldLoaded);

            FRequestPlaySessionParams PlaySessionParams;
            GEditor->RequestPlaySession(PlaySessionParams);
            GEditor->StartQueuedPlaySessionRequest();
        }

        const double StartTime = FPlatformTime::Seconds();
        FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Done, StartTime](float)
        {
            UWorld* World = GetGameWorld();
            bool bWaiting = !(World && World->HasBegunPlay()) && FPlatformTime::Seconds() - StartTime < WORLD_LOAD_TIMEOUT;
            if (!bWaiting) Done.Execute();
            return bWaiting;
        }));
    }

    AFirearmsWeapon* GetSpawnedWeaponInWorld()
    {
        UWorld* World = GetGameWorld();
        if (!TestNotNull("Current test world created", World)) return nullptr;

        const UBlueprint* FirearmsWeaponBlueprint = LoadObject<UBlueprint>(nullptr, *FString(WEAPON_TO_TEST_PATH));
        if (!TestNotNull("Firearms blueprint class loaded", FirearmsWeaponBlueprint)) return nullptr;

        TSubclassOf<AFirearmsWeapon> FirearmsClass = Cast<UClass, AFirearmsWeapon>(FirearmsWeaponBlueprint->GeneratedClass.Get());
        if (!TestNotNull("Firearms class loaded", FirearmsClass.Get())) return nullptr;

        auto FirearmsWeapon = World->SpawnActor<AFirearmsWeapon>(FirearmsClass);
        TestNotNull("Firearms weapon instance created", FirearmsWeapon);

        return FirearmsWeapon;
    }

END_DEFINE_SPEC(FFirearmsWeaponTests);

void FFirearmsWeaponTests::Define()
{
    LatentBeforeEach([this](const FDoneDelegate& Done) { OpenTestMap(Done); });

    It("AmmoCheck", [this]
    {
        AFirearmsWeapon* FirearmsWeapon = GetSpawnedWeaponInWorld();
        if (!FirearmsWeapon) return;

        TestTrue("Firearms ammo should be empty by default", FirearmsWeapon->GetIsAmmoEmpty());
        TestFalse("Firearms ammo should be not full by default", FirearmsWeapon->GetIsAmmoFull());

        FirearmsWeapon->RefillAmmo(FirearmsWeapon->GetMaxAmmo());

        TestFalse("Firearms ammo should be empty after refill", FirearmsWeapon->GetIsAmmoEmpty());
        TestTrue("Firearms ammo should be full after refill", FirearmsWeapon->GetIsAmmoFull());
    });

    It("PerformsShot", [this]
    {
        AFirearmsWeapon* FirearmsWeapon = GetSpawnedWeaponInWorld();
        if (!FirearmsWeapon) return;

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
