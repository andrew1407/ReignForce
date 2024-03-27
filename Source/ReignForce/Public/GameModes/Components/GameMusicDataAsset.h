// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameMusicDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UGameMusicDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TArray<TSoftObjectPtr<USoundBase>> GamepayMusic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TArray<TSoftObjectPtr<USoundBase>> MainMenuMusic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TArray<TSoftObjectPtr<USoundBase>> SkillsMenuMusic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TArray<TSoftObjectPtr<USoundBase>> PauseMenuMusic;
};
