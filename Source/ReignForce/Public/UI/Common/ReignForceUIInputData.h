// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonInputBaseTypes.h"
#include "ReignForceUIInputData.generated.h"


class UDataTable;

/**
 * Common UI default click & back actions (keyboard + gamepad), referenced by CommonInputSettings.InputData
 */
UCLASS()
class REIGNFORCE_API UReignForceUIInputData : public UCommonUIInputData
{
	GENERATED_BODY()

public:
	UReignForceUIInputData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

private:
	UPROPERTY()
	TObjectPtr<UDataTable> UIActions;
};
