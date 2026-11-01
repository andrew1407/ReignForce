#pragma once

#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"


class AShooterCharacter;
class APlayerCharacter;

namespace TestUtils
{
    constexpr const TCHAR* TEST_MAP_PATH = TEXT("/Game/Core/Maps/Test/TestMap");
    constexpr const TCHAR* TEST_MAP_NAME = TEXT("TestMap");
    constexpr const TCHAR* MAIN_GAME_MAP_PATH = TEXT("/Game/Core/Maps/MainGame/MainGameMap");
    constexpr const TCHAR* MAIN_GAME_MAP_NAME = TEXT("MainGameMap");
    constexpr const TCHAR* PLAYER_BLUEPRINT_PATH = TEXT("/Game/ThirdPerson/Blueprints/BP_PlayerCharacter.BP_PlayerCharacter");
    constexpr const TCHAR* ENEMY_BLUEPRINT_PATH = TEXT("/Game/ThirdPerson/Blueprints/BP_EnemyCharacter.BP_EnemyCharacter");
    constexpr const TCHAR* PISTOL_BLUEPRINT_PATH = TEXT("/Game/Core/Weapons/BP_Pistol.BP_Pistol");
    constexpr const TCHAR* MACHETE_BLUEPRINT_PATH = TEXT("/Game/Core/Weapons/BP_Machete.BP_Machete");
    constexpr double WORLD_LOAD_TIMEOUT = 30.0;

    /** First PIE (or game) world */
    UWorld* GetGameWorld();

    /**
     * Starts a PIE session of the test map (unless it's already running) and completes when the world has begun play:
     * AutomationOpenMap queues the editor PIE start behind the spec's own latent steps (UE 5.8+), so it's started directly
     */
    void OpenTestMap(FAutomationTestBase& Test, const FDoneDelegate& Done);

    /** Starts a PIE session of any map (unless it's the running one) and completes when its world has begun play */
    void OpenMap(FAutomationTestBase& Test, const TCHAR* MapPath, const TCHAR* MapName, const FDoneDelegate& Done);

    /** The test map is empty: a wide static floor (top at Z = 0) is spawned once per PIE session for characters to stand & run on */
    AActor* EnsureTestFloor(UWorld* World);

    /** Ends the running PIE session (spec's AfterEach of the world tests) */
    void CloseGameWorld();

    /** Completes when the condition holds or the timeout is out (checked every engine tick) */
    void WaitUntil(const FDoneDelegate& Done, TFunction<bool()> Condition, double Timeout);

    /** Completes after the given game time is simulated */
    void WaitFor(const FDoneDelegate& Done, double Seconds);

    /** Generated class of a blueprint asset (BP object path) */
    UClass* LoadBlueprintClass(const TCHAR* BlueprintPath);

    /** Spawns a blueprint actor always (collisions adjusted), nullptr on failure */
    AActor* SpawnBlueprintActor(UWorld* World, const TCHAR* BlueprintPath, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    /** Player character possessed by the first local player controller */
    AShooterCharacter* GetPlayerShooter(UWorld* World);

    /** The possessed player character, spawned (player blueprint) & possessed if the map has none (the test map has no player start pawn) */
    AShooterCharacter* EnsurePlayerShooter(UWorld* World, const FVector& Location = FVector(0, 0, 150));

    /** Reflection access to (private) properties by name: tests drive components without exposing their internals */
    template <typename TValue>
    TValue* GetPropertyPtr(UObject* Object, const FName& PropertyName)
    {
        if (!IsValid(Object)) return nullptr;
        FProperty* Property = Object->GetClass()->FindPropertyByName(PropertyName);
        return Property ? Property->ContainerPtrToValuePtr<TValue>(Object) : nullptr;
    }

    template <typename TValue>
    bool SetPropertyValue(UObject* Object, const FName& PropertyName, const TValue& Value)
    {
        TValue* ValuePtr = GetPropertyPtr<TValue>(Object, PropertyName);
        if (ValuePtr) *ValuePtr = Value;
        return !!ValuePtr;
    }
} // namespace TestUtils

#endif
