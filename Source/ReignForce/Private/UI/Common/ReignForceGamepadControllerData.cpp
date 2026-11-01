// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/ReignForceGamepadControllerData.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"



namespace
{
    constexpr const TCHAR* ICONS_DIR = TEXT("/Game/Core/UI/Gamepad/");

    TSoftObjectPtr<UTexture2D> Icon(const TCHAR* Name)
    {
        const FString Path = FString::Printf(TEXT("%sT_Gamepad_%s.T_Gamepad_%s"), ICONS_DIR, Name, Name);
        return TSoftObjectPtr<UTexture2D>(FSoftObjectPath(Path));
    }
}

UReignForceGamepadControllerData::UReignForceGamepadControllerData(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    InputType = ECommonInputType::Gamepad;
    GamepadName = TEXT("Generic");
    GamepadDisplayName = FText::FromString(TEXT("Gamepad"));

    // Dark badges with a light outline (readable over the orange menu buttons & the dark panels), Xbox layout:
    // face buttons, D-pad (pressed arm filled), sticks, triggers, bumpers, view & menu buttons
    KeyIcons = {
        { EKeys::Gamepad_FaceButton_Bottom, Icon(TEXT("A")) },
        { EKeys::Gamepad_FaceButton_Right, Icon(TEXT("B")) },
        { EKeys::Gamepad_FaceButton_Left, Icon(TEXT("X")) },
        { EKeys::Gamepad_FaceButton_Top, Icon(TEXT("Y")) },
        { EKeys::Gamepad_DPad_Up, Icon(TEXT("DPad_Up")) },
        { EKeys::Gamepad_DPad_Down, Icon(TEXT("DPad_Down")) },
        { EKeys::Gamepad_DPad_Left, Icon(TEXT("DPad_Left")) },
        { EKeys::Gamepad_DPad_Right, Icon(TEXT("DPad_Right")) },
        { EKeys::Gamepad_Left2D, Icon(TEXT("LS")) },
        { EKeys::Gamepad_LeftThumbstick, Icon(TEXT("LS")) },
        { EKeys::Gamepad_Right2D, Icon(TEXT("RS")) },
        { EKeys::Gamepad_RightThumbstick, Icon(TEXT("RS")) },
        { EKeys::Gamepad_LeftTrigger, Icon(TEXT("LT")) },
        { EKeys::Gamepad_LeftTriggerAxis, Icon(TEXT("LT")) },
        { EKeys::Gamepad_RightTrigger, Icon(TEXT("RT")) },
        { EKeys::Gamepad_RightTriggerAxis, Icon(TEXT("RT")) },
        { EKeys::Gamepad_LeftShoulder, Icon(TEXT("LB")) },
        { EKeys::Gamepad_RightShoulder, Icon(TEXT("RB")) },
        { EKeys::Gamepad_Special_Left, Icon(TEXT("View")) },
        { EKeys::Gamepad_Special_Right, Icon(TEXT("Menu")) },
    };
}

bool UReignForceGamepadControllerData::TryGetInputBrush(FSlateBrush& OutBrush, const FKey& Key) const
{
    const TSoftObjectPtr<UTexture2D>* IconPtr = KeyIcons.Find(Key);
    UTexture2D* Texture = IconPtr ? IconPtr->LoadSynchronous() : nullptr;
    if (!IsValid(Texture)) return Super::TryGetInputBrush(OutBrush, Key);

    OutBrush = FSlateBrush();
    OutBrush.SetResourceObject(Texture);
    OutBrush.ImageSize = FVector2D(ICON_TEXTURE_SIZE);
    OutBrush.DrawAs = ESlateBrushDrawType::Image;
    return true;
}

bool UReignForceGamepadControllerData::GetGamepadKeyBrush(const FKey& Key, FSlateBrush& OutBrush)
{
    const UCommonInputPlatformSettings* Settings = UCommonInputPlatformSettings::Get();
    if (Settings && Settings->TryGetInputBrush(OutBrush, Key, ECommonInputType::Gamepad, Settings->GetDefaultGamepadName())) return true;
    // Platform settings not set up (e.g. a platform without the controller data): this class icons are used directly
    return GetDefault<UReignForceGamepadControllerData>()->TryGetInputBrush(OutBrush, Key);
}
