// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter/Components/ReactionsComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/AssetManager.h"
#include "Components/AudioComponent.h"

#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

#include "ShooterCharacter/Components/ReactionSoundsDataAsset.h"


namespace
{
	constexpr bool bStopWhenAttachedToDestroyed = false;
}

UReactionsComponent::UReactionsComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	AttachedSocketSoundSource = TEXT("head");
}

bool UReactionsComponent::IsDataAssetValid() const
{
	return IsValid(ReactionsAsset) && ReactionsAsset->IsValidLowLevel();
}

bool UReactionsComponent::ReactionSoundSet() const
{
	return IsValid(ReactionSound);
}

bool UReactionsComponent::ReactionSoundPlaying() const
{
	return ReactionSoundSet() && ReactionSound->IsPlaying();
}

void UReactionsComponent::InterruptReactionSound()
{
	if (!ReactionSoundSet()) return;
	ReactionSound->Stop();
	ReactionSound = nullptr;
}

void UReactionsComponent::ResetReactionSound(UAudioComponent* Sound)
{
	InterruptReactionSound();
	ReactionSound = Sound;
}

UAudioComponent* UReactionsComponent::MakeOnLandedSound(const FVector& Location, const FRotator& Rotator, float ExtraVolume)
{
	if (!IsDataAssetValid()) return nullptr;
	FSoundOptions Options = ReactionsAsset->OnLandedSoundOptions;
	Options.Volume += ExtraVolume;
	return PerformSound(ReactionsAsset->OnLandedSound, Options, Location, Rotator);
}

UAudioComponent* UReactionsComponent::MakeJumpGruntSound(const FVector& Location, const FRotator& Rotator)
{
	if (!IsDataAssetValid()) return nullptr;
	return PerformSound(GetRanbdomSoundFrom(ReactionsAsset->JumpGruntSounds), ReactionsAsset->JumpGruntSoundOptions, Location, Rotator);
}

UAudioComponent* UReactionsComponent::ScreamInPain(const FVector& Location, const FRotator& Rotator, EWeaponType Cause)
{
	if (!(IsValid(GetOwner()) && IsDataAssetValid())) return nullptr;
	USoundBase* DamageTakenSound = GetRanbdomSoundFrom(ReactionsAsset->DamageTakenSounds);
	if (!IsValid(DamageTakenSound)) return nullptr;

	FSoundOptions* Options = ReactionsAsset->DamageTakenOptions.Find(Cause);
	if (!Options) Options = &ReactionsAsset->DefaultDamageOptions;
	if (Options->Chance < FMath::FRand()) return nullptr;

	auto OwnerPawn = Cast<APawn>(GetOwner());
	if (IsValid(OwnerPawn))
	{
		const FName NoiseTag = MakeNoiseTag(TEXT("DamageTaken"), Options->Tag.ToString());
		OwnerPawn->MakeNoise(
			Options->Loudness,
			OwnerPawn,
			Location,
			Options->LoudnessRange,
			NoiseTag
		);
	}

	return UGameplayStatics::SpawnSoundAttached(
        DamageTakenSound,
        GetOwnerMesh(), // Attach to the root component of the owning actor
        AttachedSocketSoundSource, // Attach to the root component, you can specify a socket or bone name if needed
        Location,
        Rotator,
        EAttachLocation::KeepWorldPosition, // Attach location type
		bStopWhenAttachedToDestroyed,
		Options->Volume
    );
}

bool UReactionsComponent::GenerateScreamInPainChance(EWeaponType Cause) const
{
	if (!IsDataAssetValid()) return true;
	constexpr float MaxChance = 1;
	const float* ProabilityPtr = ReactionsAsset->DamageScreamChances.Find(Cause);
	const float Proability = ProabilityPtr ? *ProabilityPtr : MaxChance;
	return Proability >= FMath::FRand();
}

bool UReactionsComponent::MakePerishScream(const FVector& Location, const FRotator& Rotator, bool bSetAsSoundState)
{
	if (!IsDataAssetValid()) return false;
	if (ReactionsAsset->PerishSoundOptions.Chance < FMath::FRand()) return false;
	if (ReactionsAsset->PerishSounds.IsEmpty() || !IsValid(GetOwner())) return false;

	const int32 Index = FMath::RandRange(0, ReactionsAsset->PerishSounds.Num() - 1);
	TSoftObjectPtr<USoundBase>& PerishSound = ReactionsAsset->PerishSounds[Index];

	if (PerishSound.IsNull()) return false;
	UAssetManager::GetStreamableManager().RequestAsyncLoad(PerishSound.ToSoftObjectPath(), [this, Index, Location, Rotator, bSetAsSoundState] ()
	{
		TSoftObjectPtr<USoundBase>& PerishSound = ReactionsAsset->PerishSounds[Index];
		if (!PerishSound.IsValid()) return;

		auto OwnerPawn = Cast<APawn>(GetOwner());
		if (IsValid(OwnerPawn))
		{
			const FName NoiseTag = MakeNoiseTag(TEXT("Perish"), ReactionsAsset->PerishSoundOptions.Tag.ToString());
			OwnerPawn->MakeNoise(
				ReactionsAsset->PerishSoundOptions.Loudness,
				OwnerPawn,
				Location,
				ReactionsAsset->PerishSoundOptions.LoudnessRange,
				NoiseTag
			);
		}

		UAudioComponent* Sound = UGameplayStatics::SpawnSoundAttached(
			PerishSound.Get(),
			GetOwnerMesh(), // Attach to the root component of the owning actor
			AttachedSocketSoundSource, // Attach to the root component, you can specify a socket or bone name if needed
			Location,
			Rotator,
			EAttachLocation::KeepWorldPosition, // Attach location type
			bStopWhenAttachedToDestroyed,
			ReactionsAsset->PerishSoundOptions.Volume
		);

		if (bSetAsSoundState) ResetReactionSound(Sound);
	});
	
	return true;
}

UAudioComponent* UReactionsComponent::MakeWhenHeardSound(const FVector& Location, const FRotator& Rotator)
{
	if (!IsDataAssetValid()) return nullptr;
	return PerformSound(GetRanbdomSoundFrom(ReactionsAsset->WhenHeardSounds), ReactionsAsset->WhenHeardOptions, Location, Rotator);
}

UAudioComponent* UReactionsComponent::MakeWhenSeenSound(const FVector& Location, const FRotator& Rotator)
{
	if (!IsDataAssetValid()) return nullptr;
	return PerformSound(GetRanbdomSoundFrom(ReactionsAsset->WhenSeenSounds), ReactionsAsset->WhenSeenOptions, Location, Rotator);
}

UAudioComponent* UReactionsComponent::MakeWhenLostSightSound(const FVector& Location, const FRotator& Rotator)
{
	if (!IsDataAssetValid()) return nullptr;
	return PerformSound(GetRanbdomSoundFrom(ReactionsAsset->WhenLostSightSounds), ReactionsAsset->WhenLostSightOptions, Location, Rotator);
}

UAudioComponent* UReactionsComponent::MakeJoySound(const FVector& Location, const FRotator& Rotator)
{
	if (!IsDataAssetValid()) return nullptr;
	return PerformSound(GetRanbdomSoundFrom(ReactionsAsset->JoySounds), ReactionsAsset->JoyOptions, Location, Rotator);
}

UAudioComponent* UReactionsComponent::MakeAttackGrunt(const FVector& Location, const FRotator& Rotator)
{
	if (!IsDataAssetValid()) return nullptr;
	return PerformSound(GetRanbdomSoundFrom(ReactionsAsset->AttackGrunts), ReactionsAsset->AttackGruntOptions, Location, Rotator);
}

FName UReactionsComponent::MakeNoiseTag(const FString& Type, const FString& Extra) const
{
	FString Tag = Extra;
	if (!Tag.IsEmpty()) Tag += '.';
	Tag += Type + '.';
	if (GetOwner()) Tag += GetNameSafe(GetOwner());
	return *Tag;
}

USoundBase* UReactionsComponent::GetRanbdomSoundFrom(TArray<TObjectPtr<USoundBase>> Sounds) const
{
	return Sounds.IsEmpty() ? nullptr : Sounds[FMath::RandRange(0, Sounds.Num() - 1)];
}

UAudioComponent* UReactionsComponent::PerformSound(USoundBase* Sound, const FSoundOptions& Options, const FVector& Location, const FRotator& Rotator)
{
	if (Options.Chance < FMath::FRand()) return nullptr;
	if (!(IsValid(Sound) && IsValid(GetOwner()))) return nullptr;

	auto OwnerPawn = Cast<APawn>(GetOwner());
	if (IsValid(OwnerPawn)) OwnerPawn->MakeNoise(
		Options.Loudness,
		OwnerPawn,
		Location,
		Options.LoudnessRange,
		Options.Tag
	);

	return UGameplayStatics::SpawnSoundAttached(
        Sound,
        GetOwnerMesh(), // Attach to the root component of the owning actor
        AttachedSocketSoundSource, // Attach to the root component, you can specify a socket or bone name if needed
        Location,
        Rotator,
        EAttachLocation::KeepWorldPosition, // Attach location type
		bStopWhenAttachedToDestroyed,
		Options.Volume
    );
}

USkeletalMeshComponent* UReactionsComponent::GetOwnerMesh() const
{
	if (!IsValid(GetOwner())) return nullptr;
	auto Character = Cast<ACharacter>(GetOwner());
	return IsValid(Character) ? Character->GetMesh() : nullptr;
}
