#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Button.h"

#include "Tests/TestUtils.h"
#include "UI/Skills/SkillsProgressionWidget.h"
#include "UI/Skills/SkillUpgradeWidget.h"
#include "UI/Skills/SkillDescriptionWidget.h"
#include "UI/ControlsHint/ControlsHintWidget.h"


using namespace TestUtils;

namespace
{
    const TCHAR* SKILLS_MENU_CLASS_PATH = TEXT("/Game/Core/UI/SkillsProgression/WB_SkillsProgression.WB_SkillsProgression_C");
    constexpr double SKILLS_LOAD_TIMEOUT = 10;
}

BEGIN_DEFINE_SPEC(FSkillsMenuIntegrationTests, "ReignForce.Integration.SkillsMenu",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

    TWeakObjectPtr<USkillsProgressionWidget> Menu;

    TArray<USkillUpgradeWidget*> GetSkills() const { return Menu.IsValid() ? Menu->GetSkillUpgrades() : TArray<USkillUpgradeWidget*>(); }

    int32 GetSelectedIndex() const
    {
        return GetSkills().IndexOfByPredicate([] (const USkillUpgradeWidget* Skill) { return Skill->IsSelected(); });
    }

    int32 CountSelected() const
    {
        return GetSkills().FilterByPredicate([] (const USkillUpgradeWidget* Skill) { return Skill->IsSelected(); }).Num();
    }

    UWidget* Navigate(EUINavigation Direction) const
    {
        return Menu.IsValid() ? Menu->HandleSkillNavigation(Direction) : nullptr;
    }

END_DEFINE_SPEC(FSkillsMenuIntegrationTests);

void FSkillsMenuIntegrationTests::Define()
{
    LatentBeforeEach([this](const FDoneDelegate& Done) { OpenTestMap(*this, Done); });

    Describe("Navigation", [this]
    {
        LatentBeforeEach([this](const FDoneDelegate& Done)
        {
            UWorld* World = GetGameWorld();
            if (!TestNotNull("World", World) || !TestNotNull("Player", EnsurePlayerShooter(World))) return Done.Execute();
            APlayerController* Controller = UGameplayStatics::GetPlayerController(World, 0);
            UClass* MenuClass = LoadClass<USkillsProgressionWidget>(nullptr, SKILLS_MENU_CLASS_PATH);
            if (!TestNotNull("Skills menu class", MenuClass)) return Done.Execute();
            Menu = CreateWidget<USkillsProgressionWidget>(Controller, MenuClass);
            if (!TestTrue("Skills menu created", Menu.IsValid())) return Done.Execute();
            Menu->AddToViewport(100);
            // Skill images are loaded asynchronously, the first skill gets selected with them
            WaitUntil(Done, [this] { return GetSelectedIndex() != INDEX_NONE; }, SKILLS_LOAD_TIMEOUT);
        });

        AfterEach([this]
        {
            if (Menu.IsValid()) Menu->RemoveFromParent();
            Menu.Reset();
        });

        It("SelectsTheNextSkillOnEveryStep", [this]
        {
            const TArray<USkillUpgradeWidget*> Skills = GetSkills();
            if (!TestTrue("Several skills", Skills.Num() >= 3)) return;
            TestEqual("First skill is selected on opening", GetSelectedIndex(), 0);

            for (int32 Step = 1; Step <= 2; ++Step)
            {
                UWidget* Target = Navigate(EUINavigation::Right);
                TestTrue(FString::Printf(TEXT("Right #%d moves to the next skill"), Step), Target == Skills[Step]->GetActionButton());
                TestEqual(FString::Printf(TEXT("Right #%d selects exactly the next skill"), Step), GetSelectedIndex(), Step);
                TestEqual("Single selection", CountSelected(), 1);
            }

            UWidget* Back = Navigate(EUINavigation::Left);
            TestTrue("Left moves to the previous skill", Back == Skills[1]->GetActionButton());
            TestEqual("Left selects the previous skill", GetSelectedIndex(), 1);
        });

        It("StopsAtTheFirstSkill", [this]
        {
            TestNull("Nothing before the first skill", Navigate(EUINavigation::Left));
            TestEqual("Selection stays", GetSelectedIndex(), 0);
        });

        It("ShowsBackOnTheCloseButton", [this]
        {
            UButton* Close = Menu->GetCloseMenuButton();
            if (!TestNotNull("Close button", Close)) return;
            constexpr bool bFocused = true;
            TestTrue("Close shows B", Menu->GetGamepadPromptKey(Close, !bFocused) == EKeys::Gamepad_FaceButton_Right);
            TestTrue("Focused close shows A", Menu->GetGamepadPromptKey(Close, bFocused) == EKeys::Gamepad_FaceButton_Bottom);
        });
    });

    Describe("ControlsHint", [this]
    {
        It("SwitchesTheTableWithTheInputDevice", [this]
        {
            UWorld* World = GetGameWorld();
            APlayerController* Controller = IsValid(World) ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
            if (!TestNotNull("Player controller", Controller)) return;
            UControlsHintWidget* Hint = CreateWidget<UControlsHintWidget>(Controller, UControlsHintWidget::StaticClass());
            if (!TestNotNull("Controls hint", Hint)) return;
            TestTrue("Waits before sliding in", Hint->GetPhase() == EControlsHintPhase::Waiting);

            Hint->AddToViewport();
            Hint->ShowInputDevice(true);
            TestTrue("Gamepad table", Hint->IsShowingGamepad());
            Hint->ShowInputDevice(false);
            TestFalse("Keyboard & mouse table", Hint->IsShowingGamepad());
            Hint->RemoveFromParent();
        });

        It("SlidesInStaysAndLeaves", [this]
        {
            UWorld* World = GetGameWorld();
            APlayerController* Controller = IsValid(World) ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
            if (!TestNotNull("Player controller", Controller)) return;
            UControlsHintWidget* Hint = CreateWidget<UControlsHintWidget>(Controller, UControlsHintWidget::StaticClass());
            if (!TestNotNull("Controls hint", Hint)) return;
            const float StartDelay = *GetPropertyPtr<float>(Hint, TEXT("StartDelay"));
            const float SlideDuration = *GetPropertyPtr<float>(Hint, TEXT("SlideDuration"));
            const float ShowDuration = *GetPropertyPtr<float>(Hint, TEXT("ShowDuration"));
            TestTrue("Shown for 5 seconds", FMath::IsNearlyEqual(ShowDuration, 5.f));
            Hint->AddToViewport();

            // Widgets don't tick in a headless session: the timeline is stepped like the ticks would do (in 20 ms frames),
            // each phase lasts its duration (within a couple of frames)
            constexpr float Frame = .02f;
            auto StepThrough = [this, Hint] (EControlsHintPhase Phase, EControlsHintPhase Next, float Duration)
            {
                if (!TestTrue(FString::Printf(TEXT("In phase %d"), static_cast<int32>(Phase)), Hint->GetPhase() == Phase)) return;
                float Elapsed = 0;
                while (Hint->GetPhase() == Phase && Elapsed < Duration + 1) { Hint->Advance(Frame); Elapsed += Frame; }
                TestTrue(FString::Printf(TEXT("Phase %d -> %d"), static_cast<int32>(Phase), static_cast<int32>(Next)), Hint->GetPhase() == Next);
                TestTrue(FString::Printf(TEXT("Phase %d lasts %.2f s (was %.2f s)"), static_cast<int32>(Phase), Duration, Elapsed),
                    FMath::IsNearlyEqual(Elapsed, Duration, Frame * 2.5f));
            };
            StepThrough(EControlsHintPhase::Waiting, EControlsHintPhase::SlidingIn, StartDelay);
            StepThrough(EControlsHintPhase::SlidingIn, EControlsHintPhase::Shown, SlideDuration);
            StepThrough(EControlsHintPhase::Shown, EControlsHintPhase::SlidingOut, ShowDuration);
            StepThrough(EControlsHintPhase::SlidingOut, EControlsHintPhase::Finished, SlideDuration);
            TestFalse("Removed from the screen", Hint->IsInViewport());
        });
    });
}

#endif
