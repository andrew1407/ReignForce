#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetTree.h"
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
        if (Menu.IsValid()) Menu->ChangeOption(UGameSettingsWidget::VSync, Delta, bWrap);
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

            Menu->HandleApplyClicked();
            TestTrue("Settings menu stays open after applying", Menu.IsValid() && Menu->IsInViewport());
            TestFalse("Applied values are the new baseline: nothing to apply", IsApplyEnabled());

            // Restores the user's VSync setting
            StepVSync(Revert);
            TestTrue("Changing back is a change again", IsApplyEnabled());
            Menu->HandleApplyClicked();
            TestFalse("Restored", IsApplyEnabled());
        });
    });
}

#endif
