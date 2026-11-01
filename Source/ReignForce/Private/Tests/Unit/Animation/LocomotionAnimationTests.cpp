#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"

#include "ShooterCharacter/Animation/ShooterAnimInstance.h"
#include "Weapons/Components/WeaponSlotsSystem.h"
#include "Weapons/WeaponType.h"


namespace
{
    constexpr float DIRECTION_TOLERANCE = 1e-4f;
    const float DIAGONAL = FMath::Sqrt(.5f);

    const TCHAR* JOG_BLEND_SPACE_PATH = TEXT("/Game/Characters/Player/Animations/JogBlendSpace.JogBlendSpace");

    /** Picks the strafes hemisphere for a direction from scratch (no previous state), like the first update of a movement */
    bool IsBackward(const FVector2D& Direction) { return UShooterAnimInstance::ShouldUseBackwardStrafes(Direction, false); }

    FVector2D Map(const FVector2D& Direction) { return UShooterAnimInstance::ToBlendSpaceDirection(Direction, IsBackward(Direction)); }

    /** Undoes the backward hemisphere shift (its diamond is centered behind the forward one) */
    FVector2D Unshift(const FVector2D& Mapped, bool bBackward)
    {
        return bBackward ? FVector2D(Mapped.X + UShooterAnimInstance::BACKWARD_STRAFES_OFFSET, Mapped.Y) : Mapped;
    }

    /** The jog blend space hull: |forward| + |right| <= 1 (idle in the center, cardinal jogs in the diamond corners) */
    float DiamondNorm(const FVector2D& Direction) { return FMath::Abs(Direction.X) + FMath::Abs(Direction.Y); }

    struct FDiagonalBlend
    {
        const TCHAR* Name;
        FVector2D Direction;
        // The jogs a diagonal is allowed to blend: the strafe of its hemisphere leads with the same leg as the forward / backward jog
        TArray<FString> Animations;
    };

    const TArray<FDiagonalBlend> DIAGONAL_BLENDS =
    {
        { TEXT("Forward right"), { FMath::Sqrt(.5f), FMath::Sqrt(.5f) }, { TEXT("Jog_Fwd_Rifle"), TEXT("Jog_Rt_Rifle") } },
        { TEXT("Forward left"), { FMath::Sqrt(.5f), -FMath::Sqrt(.5f) }, { TEXT("Jog_Fwd_Rifle"), TEXT("Jog_Lt_Rifle_Fwd") } },
        { TEXT("Backward right"), { -FMath::Sqrt(.5f), FMath::Sqrt(.5f) }, { TEXT("Jog_Bwd_Rifle"), TEXT("Jog_Rt_Rifle_Bwd") } },
        { TEXT("Backward left"), { -FMath::Sqrt(.5f), -FMath::Sqrt(.5f) }, { TEXT("Jog_Bwd_Rifle"), TEXT("Jog_Lt_Rifle") } },
    };
}

BEGIN_DEFINE_SPEC(FLocomotionAnimationTests, "ReignForce.Unit.LocomotionAnimation",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

    void TestDirection(const FString& What, const FVector2D& Input, const FVector2D& Expected)
    {
        const FVector2D Result = Map(Input);
        TestTrue(FString::Printf(TEXT("%s: %s -> %s (expected %s)"), *What, *Input.ToString(), *Result.ToString(), *Expected.ToString()),
            Result.Equals(Expected, DIRECTION_TOLERANCE));
    }

END_DEFINE_SPEC(FLocomotionAnimationTests);

void FLocomotionAnimationTests::Define()
{
    Describe("BlendSpaceDirection", [this]
    {
        It("KeepsIdleAndCardinals", [this]
        {
            TestDirection("Idle", FVector2D::ZeroVector, FVector2D::ZeroVector);
            TestDirection("Forward", { 1, 0 }, { 1, 0 });
            TestDirection("Backward", { -1, 0 }, { -1 - UShooterAnimInstance::BACKWARD_STRAFES_OFFSET, 0 });
            TestDirection("Left", { 0, -1 }, { 0, -1 });
            TestDirection("Right", { 0, 1 }, { 0, 1 });
            TestDirection("Half speed forward", { .5f, 0 }, { .5f, 0 });
        });

        It("PutsFullSpeedDiagonalsOnTheJogsEdge", [this]
        {
            // Every diagonal quadrant is mapped the same way: forward-left & backward-right used to break while the others worked
            const float BackwardX = -.5f - UShooterAnimInstance::BACKWARD_STRAFES_OFFSET;
            TestDirection("Forward right", { DIAGONAL, DIAGONAL }, { .5f, .5f });
            TestDirection("Forward left", { DIAGONAL, -DIAGONAL }, { .5f, -.5f });
            TestDirection("Backward right", { -DIAGONAL, DIAGONAL }, { BackwardX, .5f });
            TestDirection("Backward left", { -DIAGONAL, -DIAGONAL }, { BackwardX, -.5f });
        });

        It("IsSymmetricForAllQuadrants", [this]
        {
            for (float Angle = 0; Angle < 360; Angle += 7.5f)
            {
                const float Radians = FMath::DegreesToRadians(Angle);
                const FVector2D Direction(FMath::Cos(Radians), FMath::Sin(Radians));
                const bool bBackward = IsBackward(Direction);
                const FVector2D Result = Unshift(Map(Direction), bBackward);
                const FVector2D Mirrored = Unshift(Map({ Direction.X, -Direction.Y }), bBackward);
                // The hysteresis band right behind the lateral axis plays a forward strafe as is
                const bool bLateralBand = !bBackward && Direction.X < 0;

                if (!bLateralBand)
                {
                    TestEqual(FString::Printf(TEXT("Full speed at %.1f deg is on the diamond edge"), Angle), DiamondNorm(Result), 1.f, DIRECTION_TOLERANCE);
                    TestTrue(FString::Printf(TEXT("Direction at %.1f deg is kept (only scaled)"), Angle),
                        FMath::IsNearlyEqual(Result.X * Direction.Y, Result.Y * Direction.X, DIRECTION_TOLERANCE) && Result.Dot(Direction) > 0);
                }
                TestTrue(FString::Printf(TEXT("Left / right mirrored at %.1f deg"), Angle), Mirrored.Equals({ Result.X, -Result.Y }, DIRECTION_TOLERANCE));
            }
        });

        It("ScalesJogWeightBySpeed", [this]
        {
            TestEqual("Half speed diagonal blends half idle", DiamondNorm(Map({ DIAGONAL / 2, -DIAGONAL / 2 })), .5f, DIRECTION_TOLERANCE);
            TestEqual("Half speed backward diagonal blends half idle", DiamondNorm(Unshift(Map({ -DIAGONAL / 2, DIAGONAL / 2 }), true)), .5f, DIRECTION_TOLERANCE);
            TestEqual("Overspeed is clamped to full jog", DiamondNorm(Map({ 2, 2 })), 1.f, DIRECTION_TOLERANCE);
        });
    });

    Describe("StrafesHemisphere", [this]
    {
        It("SwitchesByMovementSide", [this]
        {
            TestFalse("Forward", IsBackward({ 1, 0 }));
            TestFalse("Forward diagonal", IsBackward({ DIAGONAL, -DIAGONAL }));
            TestTrue("Backward", IsBackward({ -1, 0 }));
            TestTrue("Backward diagonal", IsBackward({ -DIAGONAL, DIAGONAL }));
        });

        It("HasHysteresisAroundPureStrafe", [this]
        {
            TestFalse("Pure strafe after forward stays forward", UShooterAnimInstance::ShouldUseBackwardStrafes({ 0, 1 }, false));
            TestFalse("Pure strafe after backward returns forward", UShooterAnimInstance::ShouldUseBackwardStrafes({ 0, -1 }, true));
            TestFalse("Slight backward drift keeps forward strafes", UShooterAnimInstance::ShouldUseBackwardStrafes({ -.1f, 1 }, false));
            TestTrue("Slight backward drift keeps backward strafes", UShooterAnimInstance::ShouldUseBackwardStrafes({ -.1f, 1 }, true));
            TestTrue("Stopping keeps the backward hemisphere", UShooterAnimInstance::ShouldUseBackwardStrafes(FVector2D::ZeroVector, true));
            TestFalse("Stopping keeps the forward hemisphere", UShooterAnimInstance::ShouldUseBackwardStrafes(FVector2D::ZeroVector, false));
        });
    });

    Describe("JogBlendSpace", [this]
    {
        It("BlendsDiagonalsWithMatchingLeadLegStrafes", [this]
        {
            UBlendSpace* BlendSpace = LoadObject<UBlendSpace>(nullptr, JOG_BLEND_SPACE_PATH);
            if (!TestNotNull("Jog blend space", BlendSpace)) return;

            for (const FDiagonalBlend& Diagonal : DIAGONAL_BLENDS)
            {
                for (const float Speed : { 1.f, .6f })
                {
                    const FVector2D Input = Map(Diagonal.Direction * Speed);
                    TArray<FBlendSampleData> Samples;
                    int32 TriangulationIndex = INDEX_NONE;
                    const bool bSampled = BlendSpace->GetSamplesFromBlendInput(FVector(Input, 0), Samples, TriangulationIndex, true);
                    if (!TestTrue(FString::Printf(TEXT("%s samples"), Diagonal.Name), bSampled)) continue;

                    TSet<FString> Used;
                    float JogsWeight = 0;
                    for (const FBlendSampleData& Sample : Samples)
                    {
                        if (Sample.TotalWeight < .01f || !Sample.Animation) continue;
                        const FString Name = Sample.Animation->GetName();
                        if (Name.StartsWith(TEXT("Idle"))) continue;
                        Used.Add(Name);
                        JogsWeight += Sample.TotalWeight;
                        TestTrue(FString::Printf(TEXT("%s at %.1f speed blends %s (%.2f) of its hemisphere"), Diagonal.Name, Speed, *Name, Sample.TotalWeight),
                            Diagonal.Animations.Contains(Name));
                    }
                    for (const FString& Expected : Diagonal.Animations)
                        TestTrue(FString::Printf(TEXT("%s at %.1f speed blends %s"), Diagonal.Name, Speed, *Expected), Used.Contains(Expected));
                    TestEqual(FString::Printf(TEXT("%s at %.1f speed jogs weight"), Diagonal.Name, Speed), JogsWeight, Speed, .05f);
                }
            }
        });

        It("HasFootSyncMarkersOnEveryJog", [this]
        {
            UBlendSpace* BlendSpace = LoadObject<UBlendSpace>(nullptr, JOG_BLEND_SPACE_PATH);
            if (!TestNotNull("Jog blend space", BlendSpace)) return;

            for (const FBlendSample& Sample : BlendSpace->GetBlendSamples())
            {
                const UAnimSequence* Sequence = Cast<UAnimSequence>(Sample.Animation);
                if (!Sequence || Sequence->GetName().StartsWith(TEXT("Idle"))) continue;
                int32 Left = 0, Right = 0;
                for (const FAnimSyncMarker& Marker : Sequence->AuthoredSyncMarkers)
                {
                    Left += Marker.MarkerName == TEXT("L");
                    Right += Marker.MarkerName == TEXT("R");
                }
                TestEqual(Sequence->GetName() + TEXT(" left foot markers"), Left, 2);
                TestEqual(Sequence->GetName() + TEXT(" right foot markers"), Right, 2);
            }
        });
    });

    Describe("WeaponTypes", [this]
    {
        It("RecognizesFirearms", [this]
        {
            TestTrue("Rifle", UWeaponSlotsSystem::IsFirearmsType(EWeaponType::Rifle));
            TestTrue("Pistol", UWeaponSlotsSystem::IsFirearmsType(EWeaponType::Pistol));
            TestFalse("Melee", UWeaponSlotsSystem::IsFirearmsType(EWeaponType::Melee));
            TestFalse("Unarmed", UWeaponSlotsSystem::IsFirearmsType(EWeaponType::Unarmed));
            TestFalse("None", UWeaponSlotsSystem::IsFirearmsType(EWeaponType::None));
        });
    });
}

#endif
