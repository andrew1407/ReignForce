#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"

#include "Settings/ReignForceGameUserSettings.h"
#include "Settings/ReignForceSettings.h"
#include "UI/ControlsHint/ControlsHintWidget.h"
#include "PlayerCharacter/PlayerCharacter.h"
#include "Weapons/Types/FirearmsWeapon.h"
#include "Weapons/Types/MeleeWeapon.h"


BEGIN_DEFINE_SPEC(FControlsInputTests, "ReignForce.Unit.ControlsInput",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

    const UInputMappingContext* Context = nullptr;

    void TestKeys(FName Action, bool bGamepad, int32 MaxKeys, const TArray<FKey>& Expected)
    {
        const TArray<FKey> Keys = UControlsHintWidget::FindActionKeys(Context, Action, bGamepad, MaxKeys);
        const FString What = Action.ToString() + (bGamepad ? TEXT(" gamepad") : TEXT(" keyboard & mouse"));
        if (!TestEqual(What + TEXT(" keys count"), Keys.Num(), Expected.Num())) return;
        for (int32 i = 0; i < Keys.Num(); ++i) TestTrue(What + FString::Printf(TEXT(" key %d is %s"), i, *Expected[i].ToString()), Keys[i] == Expected[i]);
    }

    /** The mapping's modifiers applied to a raw stick value (dead zone, curve, scale...) like the enhanced input does */
    FVector2D ApplyStickModifiers(FName Action, const FKey& Key, const FVector2D& Raw, float DeltaTime)
    {
        FInputActionValue Value(Raw);
        if (!Context) return FVector2D::ZeroVector;
        for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
        {
            if (!(IsValid(Mapping.Action) && Mapping.Action->GetFName() == Action && Mapping.Key == Key)) continue;
            for (UInputModifier* Modifier : Mapping.Modifiers)
            {
                if (IsValid(Modifier)) Value = Modifier->ModifyRaw(nullptr, Value, DeltaTime);
            }
            return Value.Get<FVector2D>();
        }
        AddError(FString::Printf(TEXT("No %s mapping of %s"), *Key.ToString(), *Action.ToString()));
        return FVector2D::ZeroVector;
    }

END_DEFINE_SPEC(FControlsInputTests);

void FControlsInputTests::Define()
{
    Describe("ControlsHintKeys", [this]
    {
        BeforeEach([this]
        {
            Context = UReignForceSettings::Get()->RemappableInputContext.LoadSynchronous();
            TestNotNull("Gameplay mapping context", Context);
        });

        It("ShowsMovementKeysInReadingOrder", [this]
        {
            TestKeys(TEXT("IA_Move"), false, 4, { EKeys::W, EKeys::A, EKeys::S, EKeys::D });
            TestKeys(TEXT("IA_Move"), true, 4, { EKeys::Gamepad_Left2D });
        });

        It("ShowsTheMainActionsGamepadButtons", [this]
        {
            TestKeys(TEXT("IA_Aim"), true, 1, { EKeys::Gamepad_LeftTrigger });
            TestKeys(TEXT("IA_Attack"), true, 1, { EKeys::Gamepad_RightTrigger });
            TestKeys(TEXT("IA_ToggleSkillsMenu"), true, 1, { EKeys::Gamepad_Special_Left });
            TestKeys(TEXT("IA_SaveGameProgress"), true, 1, { EKeys::Gamepad_RightShoulder });
            TestKeys(TEXT("IA_TogglePause"), true, 2, { EKeys::Gamepad_Special_Right });
        });

        It("ShowsTheMainActionsKeyboardKeys", [this]
        {
            TestKeys(TEXT("IA_Aim"), false, 1, { EKeys::RightMouseButton });
            TestKeys(TEXT("IA_Attack"), false, 1, { EKeys::LeftMouseButton });
            TestKeys(TEXT("IA_ToggleSkillsMenu"), false, 1, { EKeys::K });
            TestKeys(TEXT("IA_TogglePause"), false, 2, { EKeys::P, EKeys::Escape });
        });

        It("AttacksWithXAndReloadsWithB", [this]
        {
            TestKeys(TEXT("IA_Attack"), true, 2, { EKeys::Gamepad_RightTrigger, EKeys::Gamepad_FaceButton_Left });
            TestKeys(TEXT("IA_ReloadWeapon"), true, 1, { EKeys::Gamepad_FaceButton_Right });
        });

        It("MovesAsFastAsTheStickIsTilted", [this]
        {
            // Movement input above 1 is clamped by the movement component: a scaled up stick would always run at full speed
            const FVector2D Half = ApplyStickModifiers(TEXT("IA_Move"), EKeys::Gamepad_Left2D, FVector2D(0, .5f), 1 / 60.f);
            const FVector2D Full = ApplyStickModifiers(TEXT("IA_Move"), EKeys::Gamepad_Left2D, FVector2D(0, 1), 1 / 60.f);
            const FVector2D Small = ApplyStickModifiers(TEXT("IA_Move"), EKeys::Gamepad_Left2D, FVector2D(.1f, 0), 1 / 60.f);
            TestTrue("Full tilt is the full speed (as a key press)", FMath::IsNearlyEqual(Full.Size(), 1.0, .01));
            TestTrue("Half tilt is slower", Half.Size() > .1 && Half.Size() < .75);
            TestTrue("Dead zone stays still", Small.IsNearlyZero());
        });

        It("TurnsTheCameraAsFastAsTheStickIsTilted", [this]
        {
            constexpr float DeltaTime = 1 / 60.f;
            const FVector2D Half = ApplyStickModifiers(TEXT("IA_Look"), EKeys::Gamepad_Right2D, FVector2D(.5f, 0), DeltaTime);
            const FVector2D Full = ApplyStickModifiers(TEXT("IA_Look"), EKeys::Gamepad_Right2D, FVector2D(1, 0), DeltaTime);
            TestTrue("Full tilt turns", FMath::Abs(Full.X) > 0);
            TestTrue("Half tilt turns much slower (precise aiming)", FMath::Abs(Half.X) > 0 && FMath::Abs(Half.X) < FMath::Abs(Full.X) * .4);
            // The full tilt turn rate is per second, not per frame
            const FVector2D FullSlowFrames = ApplyStickModifiers(TEXT("IA_Look"), EKeys::Gamepad_Right2D, FVector2D(1, 0), DeltaTime * 2);
            TestTrue("Turn scales with the frame time", FMath::IsNearlyEqual(FullSlowFrames.X, Full.X * 2, .001));
        });

        It("UsesCompactKeyCaps", [this]
        {
            TestEqual("Left mouse", UControlsHintWidget::GetKeyCapText(EKeys::LeftMouseButton).ToString(), FString(TEXT("LMB")));
            TestEqual("Escape", UControlsHintWidget::GetKeyCapText(EKeys::Escape).ToString(), FString(TEXT("Esc")));
            TestEqual("Letter", UControlsHintWidget::GetKeyCapText(EKeys::W).ToString(), FString(TEXT("W")));
        });
    });

    Describe("LookInversion", [this]
    {
        It("KeepsTheDirectionsByDefault", [this]
        {
            auto Settings = NewObject<UReignForceGameUserSettings>(GetTransientPackage());
            Settings->SetToDefaults();
            const FVector2D Input(1, -2);
            TestTrue("Mouse look unchanged", Settings->ApplyLookInversion(Input, false).Equals(Input));
            TestTrue("Stick look unchanged", Settings->ApplyLookInversion(Input, true).Equals(Input));
        });

        It("InvertsEachDeviceAxisSeparately", [this]
        {
            auto Settings = NewObject<UReignForceGameUserSettings>(GetTransientPackage());
            Settings->SetToDefaults();
            const FVector2D Input(1, -2);
            constexpr bool bGamepad = true;
            constexpr bool bVertical = true;

            Settings->SetLookInverted(!bGamepad, !bVertical, true);
            TestTrue("Mouse left / right inverted", Settings->ApplyLookInversion(Input, !bGamepad).Equals(FVector2D(-1, -2)));
            TestTrue("Stick isn't affected", Settings->ApplyLookInversion(Input, bGamepad).Equals(Input));

            Settings->SetLookInverted(bGamepad, bVertical, true);
            TestTrue("Stick up / down inverted", Settings->ApplyLookInversion(Input, bGamepad).Equals(FVector2D(1, 2)));
            TestTrue("Flag is read back", Settings->IsLookInverted(bGamepad, bVertical));
            TestFalse("Other stick axis stays", Settings->IsLookInverted(bGamepad, !bVertical));

            Settings->SetToDefaults();
            TestFalse("Defaults reset the inversion", Settings->IsLookInverted(!bGamepad, !bVertical));
        });
    });

    Describe("DamageVibration", [this]
    {
        It("TellsTheDamageSourceByItsCauser", [this]
        {
            TestTrue("Firearm hit is a bullet", APlayerCharacter::GetDamageVibrationSource(GetDefault<AFirearmsWeapon>()) == EDamageVibrationSource::Bullet);
            TestTrue("Melee weapon hit is a machete", APlayerCharacter::GetDamageVibrationSource(GetDefault<AMeleeWeapon>()) == EDamageVibrationSource::Machete);
            TestTrue("Character hit is a punch", APlayerCharacter::GetDamageVibrationSource(GetDefault<AShooterCharacter>()) == EDamageVibrationSource::Fist);
            TestTrue("No causer (fall) has no rumble", APlayerCharacter::GetDamageVibrationSource(nullptr) == EDamageVibrationSource::None);
            TestTrue("Other actors (land surface) have no rumble", APlayerCharacter::GetDamageVibrationSource(GetDefault<AActor>()) == EDamageVibrationSource::None);
        });

        It("GetsStrongerFromBulletsToMacheteToFists", [this]
        {
            const APlayerCharacter* Player = GetDefault<APlayerCharacter>();
            const FGamepadVibration* Bullet = Player->FindDamageVibration(EDamageVibrationSource::Bullet);
            const FGamepadVibration* Machete = Player->FindDamageVibration(EDamageVibrationSource::Machete);
            const FGamepadVibration* Fist = Player->FindDamageVibration(EDamageVibrationSource::Fist);
            if (!TestTrue("Every source rumbles", Bullet && Machete && Fist)) return;
            TestNull("No rumble without a source", Player->FindDamageVibration(EDamageVibrationSource::None));
            TestTrue("Bullet is the lightest", Bullet->Intensity > 0 && Bullet->Intensity < Machete->Intensity);
            TestTrue("Fist is the strongest", Machete->Intensity < Fist->Intensity && Fist->Intensity <= 1);
            TestTrue("Heavier hits rumble longer", Bullet->Duration <= Machete->Duration && Machete->Duration <= Fist->Duration);
        });
    });
}

#endif
