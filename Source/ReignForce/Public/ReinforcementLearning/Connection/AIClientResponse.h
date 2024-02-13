#pragma once

#include "Containers/UnrealString.h"
#include "AIClientResponse.generated.h"


USTRUCT(BlueprintType)
struct FAIClientResponse
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Event;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsSuccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, UIMin = 0))
    int32 Step = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Command = 0;
};