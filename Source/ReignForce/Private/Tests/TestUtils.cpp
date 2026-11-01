#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "Tests/TestUtils.h"
#include "Containers/Ticker.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "Engine/Blueprint.h"
#include "GameFramework/PlayerController.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/CollisionProfile.h"

#include "ShooterCharacter/ShooterCharacter.h"


namespace TestUtils
{
    UWorld* GetGameWorld()
    {
        const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
        for (const auto& Context : WorldContexts)
            if ((Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game) && Context.World())
                return Context.World();
        return nullptr;
    }

    void OpenMap(FAutomationTestBase& Test, const TCHAR* MapPath, const TCHAR* MapName, const FDoneDelegate& Done)
    {
        UWorld* GameWorld = GetGameWorld();
        if (!GameWorld || UWorld::RemovePIEPrefix(GameWorld->GetMapName()) != MapName)
        {
            if (GEditor->PlayWorld) GEditor->EndPlayMap();

            const bool bIsWorldLoaded = FEditorFileUtils::LoadMap(MapPath, false, false);
            Test.TestTrue(FString::Printf(TEXT("%s map loaded"), MapName), bIsWorldLoaded);

            FRequestPlaySessionParams PlaySessionParams;
            GEditor->RequestPlaySession(PlaySessionParams);
            GEditor->StartQueuedPlaySessionRequest();
        }

        WaitUntil(Done, []
        {
            UWorld* World = GetGameWorld();
            return World && World->HasBegunPlay();
        }, WORLD_LOAD_TIMEOUT);
    }

    void OpenTestMap(FAutomationTestBase& Test, const FDoneDelegate& Done)
    {
        FDoneDelegate OnBegunPlay = FDoneDelegate::CreateLambda([Done]
        {
            EnsureTestFloor(GetGameWorld());
            Done.Execute();
        });
        OpenMap(Test, TEST_MAP_PATH, TEST_MAP_NAME, OnBegunPlay);
    }

    AActor* EnsureTestFloor(UWorld* World)
    {
        static const FName FloorTag = TEXT("TestFloor");
        if (!World) return nullptr;
        for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
            if (It->ActorHasTag(FloorTag)) return *It;

        UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        if (!Cube) return nullptr;
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        // 100 units cube: 80 x 80 m wide, 1 m thick with its top at Z = 0
        auto Floor = World->SpawnActor<AStaticMeshActor>(FVector(0, 0, -50), FRotator::ZeroRotator, SpawnParameters);
        if (!Floor) return nullptr;
        Floor->Tags.Add(FloorTag);
        UStaticMeshComponent* Mesh = Floor->GetStaticMeshComponent();
        Mesh->SetMobility(EComponentMobility::Movable);
        Mesh->SetStaticMesh(Cube);
        Mesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
        Floor->SetActorScale3D(FVector(80, 80, 1));
        return Floor;
    }

    void CloseGameWorld()
    {
        if (GEditor && GEditor->PlayWorld) GEditor->RequestEndPlayMap();
    }

    void WaitUntil(const FDoneDelegate& Done, TFunction<bool()> Condition, double Timeout)
    {
        const double StartTime = FPlatformTime::Seconds();
        FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Done, StartTime, Timeout, Condition = MoveTemp(Condition)](float)
        {
            const bool bWaiting = !Condition() && FPlatformTime::Seconds() - StartTime < Timeout;
            if (!bWaiting) Done.Execute();
            return bWaiting;
        }));
    }

    void WaitFor(const FDoneDelegate& Done, double Seconds)
    {
        UWorld* World = GetGameWorld();
        const double StartTime = World ? World->GetTimeSeconds() : FPlatformTime::Seconds();
        WaitUntil(Done, [StartTime, Seconds, WeakWorld = TWeakObjectPtr<UWorld>(World)]
        {
            if (!WeakWorld.IsValid()) return true;
            return WeakWorld->GetTimeSeconds() - StartTime >= Seconds;
        }, Seconds * 4 + 5);
    }

    UClass* LoadBlueprintClass(const TCHAR* BlueprintPath)
    {
        const UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, BlueprintPath);
        return Blueprint ? Blueprint->GeneratedClass.Get() : nullptr;
    }

    AActor* SpawnBlueprintActor(UWorld* World, const TCHAR* BlueprintPath, const FVector& Location, const FRotator& Rotation)
    {
        UClass* ActorClass = LoadBlueprintClass(BlueprintPath);
        if (!World || !ActorClass) return nullptr;
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        return World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParameters);
    }

    AShooterCharacter* GetPlayerShooter(UWorld* World)
    {
        APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
        return PlayerController ? Cast<AShooterCharacter>(PlayerController->GetPawn()) : nullptr;
    }

    AShooterCharacter* EnsurePlayerShooter(UWorld* World, const FVector& Location)
    {
        if (AShooterCharacter* Player = GetPlayerShooter(World)) return Player;
        APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
        if (!PlayerController) return nullptr;
        auto Player = Cast<AShooterCharacter>(SpawnBlueprintActor(World, PLAYER_BLUEPRINT_PATH, Location));
        if (!Player) return nullptr;
        if (APawn* Previous = PlayerController->GetPawn()) Previous->Destroy();
        PlayerController->Possess(Player);
        return Player;
    }
} // namespace TestUtils

#endif
