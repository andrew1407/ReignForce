#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Components/AudioComponent.h"

#include "Tests/TestUtils.h"
#include "GameModes/LevelLoadingSubsystem.h"
#include "GameModes/Components/BackgroundMusicComponent.h"
#include "UI/ControlsHint/ControlsHintWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"


using namespace TestUtils;

BEGIN_DEFINE_SPEC(FLevelLoadingIntegrationTests, "ReignForce.Integration.LevelLoading",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

    TWeakObjectPtr<UWorld> PreviousWorld;
    TWeakObjectPtr<ULevelLoadingSubsystem> Loading;

END_DEFINE_SPEC(FLevelLoadingIntegrationTests);

void FLevelLoadingIntegrationTests::Define()
{
    LatentBeforeEach([this](const FDoneDelegate& Done) { OpenTestMap(*this, Done); });

    AfterEach([] { CloseGameWorld(); });

    Describe("OpenLevel", [this]
    {
        // Restart & go to menu are opened from the pause menu: the travel used to wait for a world timer,
        // which doesn't tick while paused, so the loading screen stayed forever
        LatentIt("TravelsWhileTheGameIsPaused", [this](const FDoneDelegate& Done)
        {
            UWorld* World = GetGameWorld();
            APlayerController* Controller = IsValid(World) ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
            Loading = ULevelLoadingSubsystem::Get(World);
            if (!TestNotNull("Player controller", Controller) || !TestTrue("Level loading subsystem", Loading.IsValid())) return Done.Execute();

            TestTrue("Game paused", Controller->SetPause(true) && World->IsPaused());
            PreviousWorld = World;
            Loading->OpenLevelAsync(TEST_MAP_NAME, true);
            TestTrue("Loading started", Loading->IsLoading());

            FDoneDelegate OnLoaded = FDoneDelegate::CreateLambda([this, Done]
            {
                UWorld* Current = GetGameWorld();
                TestTrue("A new level world was loaded", IsValid(Current) && Current != PreviousWorld.Get());
                TestFalse("Loading finished", Loading.IsValid() && Loading->IsLoading());
                TestFalse("The new level isn't paused", IsValid(Current) && Current->IsPaused());
                Done.Execute();
            });
            WaitUntil(OnLoaded, [this]
            {
                UWorld* Current = GetGameWorld();
                return IsValid(Current) && Current != PreviousWorld.Get() && Current->HasBegunPlay() && !(Loading.IsValid() && Loading->IsLoading());
            }, WORLD_LOAD_TIMEOUT);
        });

        // Restart reloads the same level without a loading screen & without the controls table,
        // levels opened from the main menu show the black & white image & the table
        LatentIt("RestartsWithoutTheLoadingImageAndControlsHint", [this](const FDoneDelegate& Done)
        {
            UWorld* World = GetGameWorld();
            Loading = ULevelLoadingSubsystem::Get(World);
            if (!TestTrue("Level loading subsystem", Loading.IsValid())) return Done.Execute();
            TestTrue("Image shown by default", Loading->IsShowingImage());
            TestFalse("A level opened directly isn't a restart", Loading->IsLevelRestarted());

            PreviousWorld = World;
            constexpr bool bRestart = true;
            Loading->OpenLevelAsync(TEST_MAP_NAME, true, FString(), bRestart);
            TestTrue("Loading started", Loading->IsLoading());
            TestFalse("No image while restarting", Loading->IsShowingImage());

            FDoneDelegate OnLoaded = FDoneDelegate::CreateLambda([this, Done]
            {
                TestTrue("Image is back for the next level openings", Loading.IsValid() && Loading->IsShowingImage());
                TestTrue("The new level knows it's a restart", Loading.IsValid() && Loading->IsLevelRestarted());
                TestTrue("Restart is found from the level world", ULevelLoadingSubsystem::IsLevelRestarted(GetGameWorld()));
                TArray<UUserWidget*> Hints;
                UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetGameWorld(), Hints, UControlsHintWidget::StaticClass(), false);
                TestEqual("No controls table on the restarted level", Hints.Num(), 0);
                Done.Execute();
            });
            WaitUntil(OnLoaded, [this]
            {
                UWorld* Current = GetGameWorld();
                return IsValid(Current) && Current != PreviousWorld.Get() && Current->HasBegunPlay() && !(Loading.IsValid() && Loading->IsLoading());
            }, WORLD_LOAD_TIMEOUT);
        });

        It("StopsTheMusic", [this]
        {
            UWorld* World = GetGameWorld();
            AGameModeBase* GameMode = IsValid(World) ? World->GetAuthGameMode() : nullptr;
            if (!TestNotNull("Game mode", GameMode)) return;

            // The test map game mode may have no music: the component is then checked on its own
            UBackgroundMusicComponent* Music = GameMode->FindComponentByClass<UBackgroundMusicComponent>();
            if (!IsValid(Music)) Music = NewObject<UBackgroundMusicComponent>(GameMode);
            TestFalse("Music isn't stopped initially", Music->IsStoppedForLevelTravel());
            Music->StopForLevelTravel();
            TestTrue("Music is stopped for the level travel", Music->IsStoppedForLevelTravel());
            TestFalse("No new tracks start while loading", Music->CanLoadMusicSound());
            TestNull("No gameplay music playing", Music->GetActiveGameplaySound() && Music->GetActiveGameplaySound()->IsPlaying() ? Music->GetActiveGameplaySound() : nullptr);
            TestNull("No pause menu music playing", Music->GetActivePauseMenuSound() && Music->GetActivePauseMenuSound()->IsPlaying() ? Music->GetActivePauseMenuSound() : nullptr);
        });
    });
}

#endif
