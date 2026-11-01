#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Misc/CoreDelegates.h"
#include "HAL/PlatformTime.h"

#include "Tests/TestUtils.h"
#include "GameStates/ShooterGameState.h"
#include "Spawners/Components/EnemySpawnerComponent.h"
#include "Spawners/Components/EnemyEquipSystem.h"
#include "ShooterCharacter/ShooterCharacter.h"
#include "GameModes/Components/BackgroundMusicComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Components/AudioComponent.h"


using namespace TestUtils;

namespace
{
    // Long enough for the level start loads (AI models, streaming) to finish: only the round start is measured
    constexpr double SETTLE_SECONDS = 8;
    constexpr double ROUND_START_TIMEOUT = 20;
    constexpr double AFTER_SPAWN_SECONDS = 8;
    // A frame (game thread, editor PIE) above it is a visible hitch
    constexpr double HITCH_MS = 250;

    /** Frame durations recorded between Start & Stop (engine frame begin to frame begin) with what the game state was after each */
    struct FFrameRecorder
    {
        TArray<double> FramesMs;
        TArray<FString> States;
        double LastFrame = 0;
        FDelegateHandle Handle;
        TFunction<FString()> SampleState;

        void Start(TFunction<FString()> InSampleState)
        {
            FramesMs.Reset();
            States.Reset();
            SampleState = MoveTemp(InSampleState);
            LastFrame = FPlatformTime::Seconds();
            Handle = FCoreDelegates::OnBeginFrame.AddLambda([this]
            {
                const double Now = FPlatformTime::Seconds();
                FramesMs.Add((Now - LastFrame) * 1000);
                States.Add(SampleState ? SampleState() : FString());
                LastFrame = Now;
            });
        }

        /** The slowest frames with the state change they brought (e.g. an enemy spawned, the music started) */
        FString DescribeSlowest(int32 Count) const
        {
            TArray<int32> Order;
            for (int32 i = 0; i < FramesMs.Num(); ++i) Order.Add(i);
            Order.Sort([this] (int32 A, int32 B) { return FramesMs[A] > FramesMs[B]; });
            FString Result;
            for (int32 i = 0; i < FMath::Min(Count, Order.Num()); ++i)
            {
                const int32 Frame = Order[i];
                const FString Before = Frame > 0 ? States[Frame - 1] : TEXT("start");
                Result += FString::Printf(TEXT("[frame %d: %.0f ms, %s -> %s] "), Frame, FramesMs[Frame], *Before, *States[Frame]);
            }
            return Result;
        }

        void Stop() { FCoreDelegates::OnBeginFrame.Remove(Handle); Handle.Reset(); }
        double Max() const { double Value = 0; for (double Ms : FramesMs) Value = FMath::Max(Value, Ms); return Value; }
    };

    template <typename TFunc>
    double TimeMs(TFunc&& Func)
    {
        const double Start = FPlatformTime::Seconds();
        Func();
        return (FPlatformTime::Seconds() - Start) * 1000;
    }
}

BEGIN_DEFINE_SPEC(FRoundStartE2ETests, "ReignForce.E2E.RoundStart",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

    TSharedPtr<FFrameRecorder> Recorder;

    AShooterGameState* GetGameState() const
    {
        UWorld* World = GetGameWorld();
        return IsValid(World) ? World->GetGameState<AShooterGameState>() : nullptr;
    }

END_DEFINE_SPEC(FRoundStartE2ETests);

void FRoundStartE2ETests::Define()
{
    LatentBeforeEach([this](const FDoneDelegate& Done)
    {
        FDoneDelegate OnOpened = FDoneDelegate::CreateLambda([Done] { WaitFor(Done, SETTLE_SECONDS); });
        OpenMap(*this, MAIN_GAME_MAP_PATH, MAIN_GAME_MAP_NAME, OnOpened);
    });

    AfterEach([this]
    {
        if (Recorder.IsValid()) Recorder->Stop();
        Recorder.Reset();
        CloseGameWorld();
    });

    Describe("Spawn", [this]
    {
        // Each step of the enemies spawn is timed apart (logged), to see which one the round start hitch comes from
        It("TimesTheSpawnSteps", [this]
        {
            AShooterGameState* GameState = GetGameState();
            if (!TestNotNull("Shooter game state", GameState)) return;
            UEnemySpawnerComponent* Spawner = GameState->GetEnemySpawnerComponent();
            UEnemyEquipSystem* Equip = GameState->GetEnemyEquipSystem();
            if (!TestNotNull("Enemy spawner", Spawner) || !TestNotNull("Enemy equip system", Equip)) return;

            FOnShooterCreatedDeferred OnCreated;
            OnCreated.BindUFunction(GameState, TEXT("EquipEnemyShooter"));

            AddInfo(FString::Printf(TEXT("Collect spawn points: %.1f ms"), TimeMs([&] { Spawner->Execute_CollectSpawnPoints(Spawner); })));
            for (int32 i = 1; i <= 3; ++i)
            {
                const double Ms = TimeMs([&] { Spawner->Execute_SpawnShooter(Spawner, OnCreated, nullptr, nullptr, 1); });
                AddInfo(FString::Printf(TEXT("Spawn enemy #%d: %.1f ms"), i, Ms));
            }
            TArray<AShooterCharacter*> Spawned = Spawner->Execute_GetSpawnedShooters(Spawner);
            TestTrue("Enemies spawned", Spawned.Num() > 0);
            AddInfo(FString::Printf(TEXT("Distribute behaviors: %.1f ms"), TimeMs([&] { Equip->Execute_DistributeBehaviors(Equip, Spawned); })));
            AddInfo(FString::Printf(TEXT("Clear spawned: %.1f ms"), TimeMs([&] { Spawner->Execute_ClearSpawnedShooters(Spawner, false); })));
        });
    });

    Describe("Round", [this]
    {
        LatentIt("StartsWithoutHitches", [this](const FDoneDelegate& Done)
        {
            AShooterGameState* GameState = GetGameState();
            if (!TestNotNull("Shooter game state", GameState)) return Done.Execute();

            Recorder = MakeShared<FFrameRecorder>();
            TWeakObjectPtr<AShooterGameState> WeakState(GameState);
            AGameModeBase* GameMode = GetGameWorld()->GetAuthGameMode();
            TWeakObjectPtr<UBackgroundMusicComponent> WeakMusic(IsValid(GameMode) ? GameMode->FindComponentByClass<UBackgroundMusicComponent>() : nullptr);
            Recorder->Start([WeakState, WeakMusic]
            {
                UEnemySpawnerComponent* Spawner = WeakState.IsValid() ? WeakState->GetEnemySpawnerComponent() : nullptr;
                const int32 Spawned = IsValid(Spawner) ? Spawner->Execute_GetSpawnedCount(Spawner) : -1;
                const bool bMusic = WeakMusic.IsValid() && IsValid(WeakMusic->GetActiveGameplaySound());
                return FString::Printf(TEXT("enemies %d%s"), Spawned, bMusic ? TEXT(" +music") : TEXT(""));
            });
            const double StartRoundMs = TimeMs([&] { GameState->StartRound(); });
            AddInfo(FString::Printf(TEXT("StartRound call: %.1f ms"), StartRoundMs));

            FDoneDelegate OnSpawned = FDoneDelegate::CreateLambda([this, Done]
            {
                AShooterGameState* State = GetGameState();
                TestTrue("Round is going", IsValid(State) && State->GetRoundState() == ERoundState::Going);

                FDoneDelegate OnSettled = FDoneDelegate::CreateLambda([this, Done]
                {
                    Recorder->Stop();
                    TArray<double> Frames = Recorder->FramesMs;
                    int32 Hitches = 0;
                    FString Slowest;
                    Frames.Sort(TGreater<double>());
                    for (int32 i = 0; i < Frames.Num(); ++i)
                    {
                        Hitches += Frames[i] > HITCH_MS;
                        if (i < 5) Slowest += FString::Printf(TEXT("%.0f "), Frames[i]);
                    }
                    AddInfo(FString::Printf(TEXT("Round start frames: %d, slowest (ms): %s"), Frames.Num(), *Slowest));
                    AddInfo(TEXT("Slowest frames: ") + Recorder->DescribeSlowest(5));
                    TestEqual(FString::Printf(TEXT("Frames over %.0f ms while the round starts"), HITCH_MS), Hitches, 0);
                    Done.Execute();
                });
                WaitFor(OnSettled, AFTER_SPAWN_SECONDS);
            });
            WaitUntil(OnSpawned, [this]
            {
                AShooterGameState* State = GetGameState();
                return !IsValid(State) || State->GetRoundState() == ERoundState::Going;
            }, ROUND_START_TIMEOUT);
        });
    });
}

#endif
