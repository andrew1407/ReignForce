// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/ReignForceUIInputData.h"
#include "Engine/DataTable.h"
#include "InputCoreTypes.h"
#include "CommonUITypes.h"


namespace
{
    // Input type infos are protected, the derived row only fills them (stored sliced as the base row struct)
    struct FUIActionRow : FCommonInputActionDataBase
    {
        FUIActionRow(const FText& InDisplayName, const FKey& KeyboardKey, const FKey& GamepadKey)
        {
            DisplayName = InDisplayName;
            KeyboardInputTypeInfo.SetKey(KeyboardKey);
            DefaultGamepadInputTypeInfo.SetKey(GamepadKey);
        }
    };

    FUIActionRow MakeUIAction(const FText& DisplayName, const FKey& KeyboardKey, const FKey& GamepadKey)
    {
        return FUIActionRow(DisplayName, KeyboardKey, GamepadKey);
    }
}

UReignForceUIInputData::UReignForceUIInputData(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    const FName ClickRow = TEXT("Click");
    const FName BackRow = TEXT("Back");

    UIActions = CreateDefaultSubobject<UDataTable>(GET_MEMBER_NAME_CHECKED(UReignForceUIInputData, UIActions));
    UIActions->RowStruct = FCommonInputActionDataBase::StaticStruct();
    UIActions->AddRow(ClickRow, MakeUIAction(FText::FromString(TEXT("Select")), EKeys::Enter, EKeys::Gamepad_FaceButton_Bottom));
    UIActions->AddRow(BackRow, MakeUIAction(FText::FromString(TEXT("Back")), EKeys::Escape, EKeys::Gamepad_FaceButton_Right));

    DefaultClickAction.DataTable = UIActions;
    DefaultClickAction.RowName = ClickRow;
    DefaultBackAction.DataTable = UIActions;
    DefaultBackAction.RowName = BackRow;
}
