// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterCharacter/Components/SoundOptions.h"
#include "Weapons/WeaponType.h"
#include "ReactionsComponent.generated.h"


class UAudioComponent;
class UReactionSoundsDataAsset;
class USkeletalMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REIGNFORCE_API UReactionsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UReactionsComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure)
	bool IsDataAssetValid() const;

	#pragma region REACTION_SOUND_STATE

	UFUNCTION(BlueprintCallable)
	bool ReactionSoundSet() const;

	UFUNCTION(BlueprintCallable)
	bool ReactionSoundPlaying() const;

	UFUNCTION(BlueprintCallable)
	void InterruptReactionSound();

	UFUNCTION(BlueprintCallable)
	void ResetReactionSound(UAudioComponent* Sound = nullptr);

	#pragma endregion

	#pragma region JUMP
	
	UFUNCTION(BlueprintCallable)
	UAudioComponent* MakeOnLandedSound(const FVector& Location, const FRotator& Rotator, float ExtraVolume = 0);

	UFUNCTION(BlueprintCallable)
	UAudioComponent* MakeJumpGruntSound(const FVector& Location, const FRotator& Rotator);

	#pragma endregion

	#pragma region DAMAGE

	UFUNCTION(BlueprintCallable)
	UAudioComponent* ScreamInPain(const FVector& Location, const FRotator& Rotator, EWeaponType Cause);

	UFUNCTION(BlueprintPure)
	bool GenerateScreamInPainChance(EWeaponType Cause) const;

	UFUNCTION(BlueprintCallable)
	bool MakePerishScream(const FVector& Location, const FRotator& Rotator, bool bSetAsSoundState = true);

	#pragma endregion

	#pragma region SENSE_REACTIONS

	UFUNCTION(BlueprintCallable)
	UAudioComponent* MakeWhenHeardSound(const FVector& Location, const FRotator& Rotator);

	UFUNCTION(BlueprintCallable)
	UAudioComponent* MakeWhenSeenSound(const FVector& Location, const FRotator& Rotator);

	UFUNCTION(BlueprintCallable)
	UAudioComponent* MakeWhenLostSightSound(const FVector& Location, const FRotator& Rotator);

	UFUNCTION(BlueprintCallable)
	UAudioComponent* MakeJoySound(const FVector& Location, const FRotator& Rotator);

	#pragma endregion

	#pragma region ATTACK_GRUNTS

	UFUNCTION(BlueprintCallable)
	UAudioComponent* MakeAttackGrunt(const FVector& Location, const FRotator& Rotator);

	#pragma endregion

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAudioComponent> ReactionSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ReactionSounds", meta = (AllowPrivateAccess = true))
	FName AttachedSocketSoundSource;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ReactionSounds|DataAssets", meta = (AllowPrivateAccess = true))
	TObjectPtr<UReactionSoundsDataAsset> ReactionsAsset;

	USkeletalMeshComponent* GetOwnerMesh() const;
	FName MakeNoiseTag(const FString& Type, const FString& Extra) const;
	USoundBase* GetRanbdomSoundFrom(TArray<TObjectPtr<USoundBase>> Sounds) const;
	UAudioComponent* PerformSound(USoundBase* Sound, const FSoundOptions& Options, const FVector& Location, const FRotator& Rotator);
};
