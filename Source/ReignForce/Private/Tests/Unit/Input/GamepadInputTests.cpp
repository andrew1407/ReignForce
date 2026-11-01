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
    const TCHAR* SWITCH_WEAPON_ACTION = TEXT("IA_SwitchWeapon");

    const FEnhancedActionKeyMapping* FindMapping(const UInputMappingContext* Context, const FKey& Key, const TCHAR* Action = nullptr)
    {
        return Context->GetMappings().FindByPredicate([&] (const FEnhancedActionKeyMapping& Mapping)
        {
            return Mapping.Key == Key && (!Action || (IsValid(Mapping.Action) && Mapping.Action->GetName() == Action));
        });
    }

    /** The weapon slot direction a pressed key gives (the switch weapon action value after the mapping modifiers) */
    FVector2D GetSlotDirection(const FEnhancedActionKeyMapping& Mapping)
    {
        FInputActionValue Value(FVector2D(1, 0));
        for (UInputModifier* Modifier : Mapping.Modifiers)
        {
            if (IsValid(Modifier)) Value = Modifier->ModifyRaw(nullptr, Value, 1 / 60.f);
        }
        return Value.Get<FVector2D>();
    }
}

BEGIN_DEFINE_SPEC(FGamepadInputTests, "ReignForce.Unit.GamepadInput",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

    const UInputMappingContext* Context = nullptr;

    void TestActionKey(const FKey& Key, const TCHAR* Action)
    {
        const FEnhancedActionKeyMapping* Mapping = FindMapping(Context, Key);
        if (!TestNotNull(Key.ToString() + TEXT(" is mapped"), Mapping)) return;
        TestTrue(FString::Printf(TEXT("%s is %s"), *Key.ToString(), Action), IsValid(Mapping->Action) && Mapping->Action->GetName() == Action);
        TestEqual(Key.ToString() + TEXT(" has a single action"),
            Context->GetMappings().FilterByPredicate([&Key] (const FEnhancedActionKeyMapping& Item) { return Item.Key == Key; }).Num(), 1);
    }

END_DEFINE_SPEC(FGamepadInputTests);

void FGamepadInputTests::Define()
{
    BeforeEach([this]
    {
        Context = LoadObject<UInputMappingContext>(nullptr, GAMEPLAY_MAPPING_CONTEXT_PATH);
        TestNotNull("Gameplay mapping context", Context);
    });

    Describe("WeaponSelection", [this]
    {
        It("SelectsEachWeaponWithItsDPadDirection", [this]
        {
            if (!Context) return;
            // Same directions as the arrow keys: the player character maps them to the weapon slots
            // (up - fists, right - rifle, down - pistol, left - machete)
            const TPair<FKey, FKey> Pairs[] = {
                { EKeys::Gamepad_DPad_Up, EKeys::Up },
                { EKeys::Gamepad_DPad_Right, EKeys::Right },
                { EKeys::Gamepad_DPad_Down, EKeys::Down },
                { EKeys::Gamepad_DPad_Left, EKeys::Left },
            };
            const FVector2D Expected[] = { FVector2D(0, 1), FVector2D(1, 0), FVector2D(0, -1), FVector2D(-1, 0) };
            for (int32 i = 0; i < UE_ARRAY_COUNT(Pairs); ++i)
            {
                TestActionKey(Pairs[i].Key, SWITCH_WEAPON_ACTION);
                const FEnhancedActionKeyMapping* Pad = FindMapping(Context, Pairs[i].Key, SWITCH_WEAPON_ACTION);
                const FEnhancedActionKeyMapping* Arrow = FindMapping(Context, Pairs[i].Value, SWITCH_WEAPON_ACTION);
                if (!Pad || !TestNotNull(Pairs[i].Value.ToString() + TEXT(" arrow mapping"), Arrow)) continue;
                const FVector2D Direction = GetSlotDirection(*Pad);
                TestTrue(Pairs[i].Key.ToString() + TEXT(" gives its slot direction"), Direction.Equals(Expected[i]));
                TestTrue(Pairs[i].Key.ToString() + TEXT(" picks the same weapon as its arrow key"), Direction.Equals(GetSlotDirection(*Arrow)));
            }
        });
    });

    Describe("Buttons", [this]
    {
        It("UsesTheBumpersAndTheMenuButtons", [this]
        {
            if (!Context) return;
            TestActionKey(EKeys::Gamepad_LeftShoulder, TEXT("IA_ResetRoundState"));
            TestActionKey(EKeys::Gamepad_RightShoulder, TEXT("IA_SaveGameProgress"));
            TestActionKey(EKeys::Gamepad_Special_Left, TEXT("IA_ToggleSkillsMenu"));
            TestActionKey(EKeys::Gamepad_Special_Right, TEXT("IA_TogglePause"));
        });

        It("HasPromptIconsForTheUsedButtons", [this]
        {
            // Focused menu buttons show the confirm button, the round start prompt the round start button
            // (the controls hint the movement stick, the triggers, the view & menu buttons)
            for (const FKey& Key : { EKeys::Gamepad_FaceButton_Bottom, EKeys::Gamepad_FaceButton_Right, EKeys::Gamepad_DPad_Down,
                EKeys::Gamepad_DPad_Up, EKeys::Gamepad_DPad_Left, EKeys::Gamepad_DPad_Right, EKeys::Gamepad_Left2D, EKeys::Gamepad_Right2D,
                EKeys::Gamepad_LeftTrigger, EKeys::Gamepad_RightTrigger, EKeys::Gamepad_LeftShoulder, EKeys::Gamepad_RightShoulder,
                EKeys::Gamepad_Special_Left, EKeys::Gamepad_Special_Right })
            {
                FSlateBrush Brush;
                const bool bFound = UReignForceGamepadControllerData::GetGamepadKeyBrush(Key, Brush);
                TestTrue(Key.ToString() + TEXT(" icon"), bFound && IsValid(Brush.GetResourceObject()));
            }
        });
    });
}

#endif
