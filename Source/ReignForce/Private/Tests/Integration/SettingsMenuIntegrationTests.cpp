#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetNavigation.h"
#include "Components/Button.h"

#include "Tests/TestUtils.h"
#include "UI/Settings/GameSettingsWidget.h"


using namespace TestUtils;

namespace
{
    const FName APPLY_BUTTON = TEXT("ApplyButton");
    const FName BACK_BUTTON = TEXT("BackButton");
}

BEGIN_DEFINE_SPEC(FSettingsMenuIntegrationTests, "ReignForce.Integration.SettingsMenu",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

    TWeakObjectPtr<UGameSettingsWidget> Menu;

    UButton* FindButton(FName Name) const
    {
        return Menu.IsValid() && Menu->WidgetTree ? Menu->WidgetTree->FindWidget<UButton>(Name) : nullptr;
    }

    bool IsApplyEnabled() const
    {
        UButton* Apply = FindButton(APPLY_BUTTON);
        return IsValid(Apply) && Apply->GetIsEnabled();
    }

    /** Steps the VSync row (two values, not previewed live) like left / right on it (D-pad or arrows) */
    void StepVSync(int32 Delta)
    {
        constexpr bool bWrap = false;
        if (Menu.IsValid()) Menu->ChangeOption(EGameSettingsOption::VSYNC, Delta, bWrap);
    }

    /** Changes the VSync row to its other value, returns the step to change it back (0 if nothing changed) */
    int32 ToggleVSync()
    {
        StepVSync(1);
        if (IsApplyEnabled()) return -1;
        // Was already the last value (values don't wrap with navigation)
        StepVSync(-1);
        return IsApplyEnabled() ? 1 : 0;
    }

END_DEFINE_SPEC(FSettingsMenuIntegrationTests);

void FSettingsMenuIntegrationTests::Define()
{
    LatentBeforeEach([this](const FDoneDelegate& Done) { OpenTestMap(*this, Done); });

    BeforeEach([this]
    {
        UWorld* World = GetGameWorld();
        APlayerController* Controller = IsValid(World) ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
        if (!TestNotNull("Player controller", Controller)) return;
        Menu = CreateWidget<UGameSettingsWidget>(Controller, UGameSettingsWidget::StaticClass());
        if (TestTrue("Settings menu created", Menu.IsValid())) Menu->AddToViewport(100);
    });

    AfterEach([this]
    {
        if (Menu.IsValid()) Menu->RemoveFromParent();
        Menu.Reset();
    });

    Describe("Apply", [this]
    {
        It("IsDisabledWithoutChanges", [this]
        {
            UButton* Apply = FindButton(APPLY_BUTTON);
            UButton* Back = FindButton(BACK_BUTTON);
            if (!TestNotNull("Apply button", Apply) || !TestNotNull("Back button", Back)) return;
            TestFalse("Apply is disabled while nothing is changed", Apply->GetIsEnabled());
            TestTrue("Back is enabled", Back->GetIsEnabled());
        });

        It("IsEnabledOnlyWhileSomethingDiffers", [this]
        {
            const int32 Revert = ToggleVSync();
            if (!TestTrue("Apply is enabled after an option change", Revert != 0)) return;
            StepVSync(Revert);
            TestFalse("Apply is disabled again once the option is back to its current value", IsApplyEnabled());
        });

        It("KeepsTheSettingsMenuOpen", [this]
        {
            const int32 Revert = ToggleVSync();
            if (!TestTrue("Apply is enabled after an option change", Revert != 0)) return;

            Menu->ApplyChanges();
            TestTrue("Settings menu stays open after applying", Menu.IsValid() && Menu->IsInViewport());
            TestFalse("Applied values are the new baseline: nothing to apply", IsApplyEnabled());

            // Restores the user's VSync setting
            StepVSync(Revert);
            TestTrue("Changing back is a change again", IsApplyEnabled());
            Menu->ApplyChanges();
            TestFalse("Restored", IsApplyEnabled());
        });
    });

    Describe("GamepadPrompts", [this]
    {
        It("ShowsBackOnTheBackButtonAndConfirmOnTheFocusedOne", [this]
        {
            UButton* Back = FindButton(BACK_BUTTON);
            UButton* Apply = FindButton(APPLY_BUTTON);
            if (!TestNotNull("Back button", Back) || !TestNotNull("Apply button", Apply)) return;
            constexpr bool bFocused = true;
            TestTrue("Back shows B while another button is focused", Menu->GetGamepadPromptKey(Back, !bFocused) == EKeys::Gamepad_FaceButton_Right);
            TestTrue("Focused Back shows A (A clicks it)", Menu->GetGamepadPromptKey(Back, bFocused) == EKeys::Gamepad_FaceButton_Bottom);
            TestFalse("Disabled Apply has no prompt", Menu->GetGamepadPromptKey(Apply, !bFocused).IsValid());
        });

        It("HidesThePromptsOfACoveredMenu", [this]
        {
            UButton* Back = FindButton(BACK_BUTTON);
            if (!TestNotNull("Back button", Back)) return;
            Menu->DeactivateWidget();
            TestFalse("No B on an inactive menu (the menu over it takes the back action)", Menu->GetGamepadPromptKey(Back, false).IsValid());
            Menu->ActivateWidget();
        });
    });

    Describe("Navigation", [this]
    {
        It("GoesDownThroughEveryOptionBeforeTheActions", [this]
        {
            // Rows out of the scrolled view are reached one by one (not skipped to Apply / Back)
            TArray<UButton*> Chain;
            for (int32 Option = 0; Option < static_cast<int32>(EGameSettingsOption::COUNT); ++Option)
            {
                UButton* Row = FindButton(*FString::Printf(TEXT("Option_%d"), Option));
                if (IsValid(Row)) Chain.Add(Row);
            }
            if (!TestTrue("Option rows found", Chain.Num() > 5)) return;
            int32 Visited = 1;
            UWidget* Current = FindButton(TEXT("Option_0"));
            TSet<UWidget*> Seen = { Current };
            while (IsValid(Current) && IsValid(Current->Navigation) && Current->Navigation->Down.Rule == EUINavigationRule::Explicit)
            {
                UWidget* Next = Current->Navigation->Down.Widget.Get();
                if (!IsValid(Next) || Seen.Contains(Next)) break;
                Seen.Add(Next);
                Current = Next;
                if (Current->GetFName() == APPLY_BUTTON || Current->GetFName() == BACK_BUTTON) break;
                ++Visited;
            }
            for (UButton* Row : Chain) TestTrue(FString::Printf(TEXT("%s is reached going down"), *Row->GetName()), Seen.Contains(Row));
            TestTrue("Down from the last item leads to the actions", IsValid(Current) && (Current->GetFName() == APPLY_BUTTON || Current->GetFName() == BACK_BUTTON));
            TestTrue("Submenu items are reached too", Visited >= Chain.Num() + 2);
        });
    });

    Describe("LookInversion", [this]
    {
        It("HasARowPerDeviceAxis", [this]
        {
            for (EGameSettingsOption Option : { EGameSettingsOption::INVERT_MOUSE_X, EGameSettingsOption::INVERT_MOUSE_Y, EGameSettingsOption::INVERT_GAMEPAD_X, EGameSettingsOption::INVERT_GAMEPAD_Y })
            {
                UButton* Row = FindButton(*FString::Printf(TEXT("Option_%d"), static_cast<int32>(Option)));
                TestNotNull(FString::Printf(TEXT("Inversion row %d"), static_cast<int32>(Option)), Row);
            }
        });

        It("EnablesApplyWhenToggled", [this]
        {
            // Two values (Off / On) that don't wrap: one of the directions changes it
            constexpr bool bWrap = false;
            Menu->ChangeOption(EGameSettingsOption::INVERT_MOUSE_Y, 1, bWrap);
            if (!IsApplyEnabled()) Menu->ChangeOption(EGameSettingsOption::INVERT_MOUSE_Y, -1, bWrap);
            const bool bChanged = IsApplyEnabled();
            // Not applied: the saved user settings stay as they were
            Menu->DiscardChanges();
            TestTrue("Inversion change can be applied", bChanged);
        });
    });
}

#endif
