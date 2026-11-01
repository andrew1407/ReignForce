// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonInputBaseTypes.h"
#include "ReignForceGamepadControllerData.generated.h"


class UTexture2D;

/**
 * Common UI gamepad button icons (generic gamepad, Xbox layout), referenced by the platforms CommonInputPlatformSettings.ControllerData:
 * Common UI widgets & the menus prompts take the key icons from it. Textures are loaded on the first request
 */
UCLASS()
class REIGNFORCE_API UReignForceGamepadControllerData : public UCommonInputBaseControllerData
{
	GENERATED_BODY()

public:
	UReignForceGamepadControllerData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool TryGetInputBrush(FSlateBrush& OutBrush, const FKey& Key) const override;

	/** Icon of a gamepad key from the active platform controller data (false when there's none) */
	static bool GetGamepadKeyBrush(const FKey& Key, FSlateBrush& OutBrush);

	/** Prompt icons are square, drawn at the given size in the layout units */
	static constexpr float ICON_TEXTURE_SIZE = 64;

	UPROPERTY(EditDefaultsOnly, Category = "Display")
	TMap<FKey, TSoftObjectPtr<UTexture2D>> KeyIcons;
};
