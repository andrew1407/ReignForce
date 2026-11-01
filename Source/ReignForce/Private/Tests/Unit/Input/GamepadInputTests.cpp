#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "InputAction.h"

#include "UI/Common/ReignForceGamepadControllerData.h"


namespace
{
    const TCHAR* GAMEPLAY_MAPPING_CONTEXT_PATH = TEXT("/Game/ThirdPerson/Input/IMC_Default.IMC_Default");
    const TCHAR* CYCLE_WEAPON_ACTION = TEXT("IA_CycleWeapon");

    const FEnhancedActionKeyMapping* FindMapping(const UInputMappingContext* Context, const FKey& Key)
    {
        return Context->GetMappings().FindByPredicate([&Key] (const FEnhancedActionKeyMapping& Mapping) { return Mapping.Key == Key; });
    }

    bool IsNegated(const FEnhancedActionKeyMapping& Mapping)
    {
        return Mapping.Modifiers.ContainsByPredicate([] (const UInputModifier* Modifier) { return IsValid(Modifier) && Modifier->IsA<UInputModifierNegate>(); });
    }
}

BEGIN_DEFINE_SPEC(FGamepadInputTests, "ReignForce.Unit.GamepadInput",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

    void TestCycleWeaponKey(const UInputMappingContext* Context, const FKey& Key, bool bPrevious)
    {
        const FEnhancedActionKeyMapping* Mapping = FindMapping(Context, Key);
        if (!TestNotNull(Key.ToString() + TEXT(" is mapped"), Mapping)) return;
        TestTrue(Key.ToString() + TEXT(" cycles weapons"), IsValid(Mapping->Action) && Mapping->Action->GetName() == CYCLE_WEAPON_ACTION);
        // The cycle direction is the action value sign: negated keys pick the previous weapon slot
        TestEqual(Key.ToString() + (bPrevious ? TEXT(" picks the previous weapon") : TEXT(" picks the next weapon")), IsNegated(*Mapping), bPrevious);
    }

END_DEFINE_SPEC(FGamepadInputTests);

void FGamepadInputTests::Define()
{
    Describe("WeaponCycling", [this]
    {
        It("UsesDPadLeftAndRight", [this]
        {
            const UInputMappingContext* Context = LoadObject<UInputMappingContext>(nullptr, GAMEPLAY_MAPPING_CONTEXT_PATH);
            if (!TestNotNull("Gameplay mapping context", Context)) return;
            TestCycleWeaponKey(Context, EKeys::Gamepad_DPad_Left, true);
            TestCycleWeaponKey(Context, EKeys::Gamepad_DPad_Right, false);
        });

        It("KeepsShoulderButtons", [this]
        {
            const UInputMappingContext* Context = LoadObject<UInputMappingContext>(nullptr, GAMEPLAY_MAPPING_CONTEXT_PATH);
            if (!TestNotNull("Gameplay mapping context", Context)) return;
            TestCycleWeaponKey(Context, EKeys::Gamepad_LeftShoulder, true);
            TestCycleWeaponKey(Context, EKeys::Gamepad_RightShoulder, false);
        });

        It("HasPromptIconsForTheUsedButtons", [this]
        {
            // Focused menu buttons show the confirm button, the round start prompt the round start button
            for (const FKey& Key : { EKeys::Gamepad_FaceButton_Bottom, EKeys::Gamepad_FaceButton_Right, EKeys::Gamepad_DPad_Down,
                EKeys::Gamepad_DPad_Up, EKeys::Gamepad_DPad_Left, EKeys::Gamepad_DPad_Right })
            {
                FSlateBrush Brush;
                const bool bFound = UReignForceGamepadControllerData::GetGamepadKeyBrush(Key, Brush);
                TestTrue(Key.ToString() + TEXT(" icon"), bFound && IsValid(Brush.GetResourceObject()));
            }
        });

        It("KeepsDPadUpAndDownForMenus", [this]
        {
            const UInputMappingContext* Context = LoadObject<UInputMappingContext>(nullptr, GAMEPLAY_MAPPING_CONTEXT_PATH);
            if (!TestNotNull("Gameplay mapping context", Context)) return;
            for (const FKey& Key : { EKeys::Gamepad_DPad_Up, EKeys::Gamepad_DPad_Down })
            {
                const FEnhancedActionKeyMapping* Mapping = FindMapping(Context, Key);
                TestTrue(Key.ToString() + TEXT(" isn't taken by weapon cycling"),
                    Mapping && IsValid(Mapping->Action) && Mapping->Action->GetName() != CYCLE_WEAPON_ACTION);
            }
        });
    });
}

#endif
