#pragma once

UENUM(BlueprintType)
enum class EShooterAICommand : uint8
{
    None UMETA(DisplayName="None"),
    Idle UMETA(DisplayName="Idle"),
    IdleAndAttack UMETA(DisplayName="IdleAndAttack"),
    GetClose UMETA(DisplayName="GetClose"),
    GetCloseAndAttack UMETA(DisplayName="GetCloseAndAttack"),
    StrafeBack UMETA(DisplayName="StrafeBack"),
    StrafeBackAndAttack UMETA(DisplayName="StrafeBackAndAttack"),
    Retreat UMETA(DisplayName="Retreat"),
};
