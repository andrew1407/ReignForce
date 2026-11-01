#pragma once

#include "CoreMinimal.h"
#include "GamepadVibration.generated.h"


USTRUCT(BlueprintType)
struct FGamepadVibration
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, ClampMax = 1, UIMax = 1))
	float Intensity = .3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, Units = "Seconds"))
	float Duration = .1f;

	/** Large (low frequency) motors give a heavy rumble, small ones - a light buzz */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLargeMotors = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSmallMotors = true;
};

/** What hit the player: the rumble gets stronger from the bullets to the machete to the fists */
UENUM(BlueprintType)
enum class EDamageVibrationSource : uint8
{
	None,
	Bullet,
	Machete,
	Fist,
};
