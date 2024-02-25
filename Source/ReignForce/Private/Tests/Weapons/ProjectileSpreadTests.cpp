#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "Tests/Weapons/ProjectileSpreadTests.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Weapons/ProjectileSpread.h"


namespace
{
    constexpr float SPREAD_DEGREE_TOLERANCE = 1e-3f; 
}

DEFINE_SPEC(FProjectileSpreadTests, "ReignForce.ProjectileSpread",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter | EAutomationTestFlags::HighPriority);

void FProjectileSpreadTests::Define()
{
    It("ConvertsToRadiansValid", [this]
    {
        constexpr float HorizontalDegreesToSet = 3;
        constexpr float VerticalDegreesToSet = 69;

        constexpr float HorizontalRadiansExpected = .052f;
        constexpr float VerticalRadiansExpected = 1.204f;

        FProjectileSpread ProjectileSpread;
        ProjectileSpread.Horizontal = HorizontalDegreesToSet;
        ProjectileSpread.Vertical = VerticalDegreesToSet;

        const float HorizontalRadiansResult = ProjectileSpread.HorizontalRadians();
        const float VerticalRadiansResult = ProjectileSpread.VerticalRadians();

        const FString LabelWhatHorizontal = FString::Printf(TEXT("Convert \"%.3f\" degrees to radians"), HorizontalDegreesToSet);
        TestEqual(LabelWhatHorizontal, HorizontalRadiansResult, HorizontalRadiansExpected, SPREAD_DEGREE_TOLERANCE);

        const FString LabelWhatVertical = FString::Printf(TEXT("Convert \"%.3f\" degrees to radians"), VerticalDegreesToSet);
        TestEqual(LabelWhatVertical, VerticalRadiansResult, VerticalRadiansExpected, SPREAD_DEGREE_TOLERANCE);
    });

    It("ChecksIfSpreadIsZero", [this]
    {
        FProjectileSpread ProjectileSpreadToBeZero;
        ProjectileSpreadToBeZero.Horizontal = 0;
        ProjectileSpreadToBeZero.Vertical = 0;

        FProjectileSpread ProjectileSpreadNotToBeZero;
        ProjectileSpreadNotToBeZero.Horizontal = 90;
        ProjectileSpreadNotToBeZero.Vertical = 90;

        const FString LabelWhatZero = FString::Printf(TEXT("Spread (%.3f, %.3f) must be zero"), ProjectileSpreadToBeZero.Horizontal, ProjectileSpreadToBeZero.Vertical);
        TestTrue(LabelWhatZero, ProjectileSpreadToBeZero.IsZero());

        const FString LabelWhatNonZero = FString::Printf(TEXT("Spread (%.3f, %.3f) must be non-zero"), ProjectileSpreadNotToBeZero.Horizontal, ProjectileSpreadNotToBeZero.Vertical);
        TestFalse(LabelWhatNonZero, ProjectileSpreadNotToBeZero.IsZero());
    });
}

#endif
