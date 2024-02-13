// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ShooterCharacter/Components/SoundOptions.h"
#include "Weapons/WeaponType.h"
#include "ReactionSoundsDataAsset.generated.h"


/**
 * 
 */
UCLASS()
class REIGNFORCE_API UReactionSoundsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UReactionSoundsDataAsset(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	#pragma region JUMP

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	TObjectPtr<USoundBase> OnLandedSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	FSoundOptions OnLandedSoundOptions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	TArray<TObjectPtr<USoundBase>> JumpGruntSounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	FSoundOptions JumpGruntSoundOptions;

	#pragma endregion

	#pragma region DAMAGE

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	TArray<TObjectPtr<USoundBase>> DamageTakenSounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	TMap<EWeaponType, FSoundOptions> DamageTakenOptions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	FSoundOptions DefaultDamageOptions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	TMap<EWeaponType, float> DamageScreamChances;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	TArray<TSoftObjectPtr<USoundBase>> PerishSounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	FSoundOptions PerishSoundOptions;

	#pragma endregion

	#pragma region SENSE_REACTIONS

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
	TArray<TObjectPtr<USoundBase>> WhenHeardSounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
	FSoundOptions WhenHeardOptions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
	TArray<TObjectPtr<USoundBase>> WhenSeenSounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
	FSoundOptions WhenSeenOptions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
	TArray<TObjectPtr<USoundBase>> WhenLostSightSounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
	FSoundOptions WhenLostSightOptions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
	TArray<TObjectPtr<USoundBase>> JoySounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reactions")
	FSoundOptions JoyOptions;

	#pragma endregion

	#pragma region ATTACK_GRUNTS

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackGrunts")
	TArray<TObjectPtr<USoundBase>> AttackGrunts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackGrunts")
	FSoundOptions AttackGruntOptions;

	#pragma endregion
};
