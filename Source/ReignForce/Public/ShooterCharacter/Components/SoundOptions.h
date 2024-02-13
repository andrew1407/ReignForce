#pragma once

#include "UObject/NameTypes.h"
#include "SoundOptions.generated.h"


USTRUCT(BlueprintType)
struct FSoundOptions
{
    GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, Units = "Times"))
	float Chance = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, Units = "Times"))
	float Volume = .6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0, Units = "Times"))
	float Loudness = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
	float LoudnessRange = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Tag;
};
