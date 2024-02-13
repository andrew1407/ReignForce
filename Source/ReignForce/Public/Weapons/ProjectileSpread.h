#pragma once

#include "Math/Vector2D.h"
#include "Math/UnrealMathUtility.h"
#include "Containers/Map.h"
#include "ProjectileSpread.generated.h"


USTRUCT(BlueprintType)
struct FProjectileSpread
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, ClampMax = 1, UIMax = 1, Units = "Times"))
    float SpreadChacne = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, ClampMax = 90, UIMax = 90, Units = "Degrees"))
    float Horizontal = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, ClampMax = 90, UIMax = 90, Units = "Degrees"))
    float Vertical = 0;

    FORCEINLINE float HorizontalRadians() const { return FMath::DegreesToRadians(Horizontal); }
    FORCEINLINE float VerticalRadians() const { return FMath::DegreesToRadians(Vertical); }

    FORCEINLINE FVector2D ToVector() const { return { Horizontal, Vertical }; }
    FORCEINLINE FVector2D ToRadianVector() const { return { HorizontalRadians(), VerticalRadians() }; }

    FORCEINLINE TPair<float, float> ToPair() const { return { Horizontal, Vertical }; }
    FORCEINLINE TPair<float, float> ToRadianPair() const { return { HorizontalRadians(), VerticalRadians() }; }

    FORCEINLINE bool IsZero() const { return (Horizontal == 0) && (Vertical == 0); }
};
