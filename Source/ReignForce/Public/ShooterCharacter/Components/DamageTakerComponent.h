// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/WeaponType.h"
#include "DamageTakerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REIGNFORCE_API UDamageTakerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDamageTakerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingDamage", meta = (ClampMin = 0, UIMin = 0))
	float MinFallDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingDamage", meta = (ClampMin = 0, UIMin = 0))
	float MaxFallDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingDamage|Velocity", meta = (ClampMin = 0, UIMin = 0))
	float MinHeightFallSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingDamage|Velocity", meta = (ClampMin = 0, UIMin = 0))
	float MaxHeightFallSpeed;

	UFUNCTION(BlueprintPure)
	EWeaponType DefineDamage(AActor* Target, TSubclassOf<AActor> PunchCauser = nullptr) const;

	UFUNCTION(BlueprintCallable)
	float ApplyFallDamage(float& DamagePercentage, float Velocity, AActor* LandSurface = nullptr);
};
