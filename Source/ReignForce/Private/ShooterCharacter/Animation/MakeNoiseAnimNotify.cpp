// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter/Animation/MakeNoiseAnimNotify.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequenceBase.h"


UMakeNoiseAnimNotify::UMakeNoiseAnimNotify(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    Loudness = 1;
    DefaultLoudnessRange = 0;
    SoundToPlayVolume = 1;
}

void UMakeNoiseAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    AActor* MeshOwner = MeshComp->GetOwner();
    if (!IsValid(MeshOwner)) return;
    const auto [ NoiseLocation, NoiseRotator ] = GetNoisePosition(MeshComp);

    if (IsValid(SoundToPlay)) UGameplayStatics::SpawnSoundAtLocation(
        GetWorld(),
        SoundToPlay,
        NoiseLocation,
        NoiseRotator,
        SoundToPlayVolume
    );

    if (DefaultLoudnessRange > 0)
        MeshOwner->MakeNoise(Loudness, Cast<APawn>(MeshOwner), NoiseLocation, DefaultLoudnessRange, NoiseTag);
}

TPair<FVector, FRotator> UMakeNoiseAnimNotify::GetNoisePosition(USkeletalMeshComponent* MeshComp) const
{
    AActor* MeshOwner = MeshComp->GetOwner();
    if (MeshSocket.IsNone()) return { MeshOwner->GetActorLocation(), MeshOwner->GetActorRotation() };
    const FTransform SocketTransform = MeshComp->GetSocketTransform(MeshSocket, ERelativeTransformSpace::RTS_World);
    bool bSocketInvalid = SocketTransform.Equals(FTransform::Identity);
    if (bSocketInvalid) return { MeshOwner->GetActorLocation(), MeshOwner->GetActorRotation() };
    return { SocketTransform.GetLocation(), SocketTransform.Rotator() };
}
