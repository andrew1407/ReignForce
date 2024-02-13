#pragma once

UENUM(BlueprintType)
enum class ERanksProgressionSource : uint8
{
    Default UMETA(DisplayName="Default"),
    FromSave UMETA(DisplayName="FromSave"),
    ProvidedBySystem UMETA(DisplayName="ProvidedBySystem"),
};
