// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Weapons/WeaponType.h"
#include "ShooterEquipmentDataAsset.generated.h"


class AWeapon;

USTRUCT(BlueprintType)
struct FWeaponModelTypes
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSoftClassPtr<AWeapon>> Models;
};

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UShooterEquipmentDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	TMap<EWeaponType, FWeaponModelTypes> WeaponModels;
};
