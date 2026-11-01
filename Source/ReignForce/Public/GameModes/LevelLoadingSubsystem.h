// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/UObjectGlobals.h"
#include "Styling/SlateBrush.h"
#include "LevelLoadingSubsystem.generated.h"


class UTexture2D;
class ULoadingScreenWidget;

/**
 * Opens levels behind a loading screen (black & white main menu image): it is shown over the current level first,
 * then the movie player keeps the same screen up (rendered on its own thread) while the map is loaded
 */
UCLASS()
class REIGNFORCE_API ULevelLoadingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static ULevelLoadingSubsystem* Get(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Falls back to the regular level opening when the subsystem isn't available */
	static void OpenLevel(const UObject* WorldContextObject, FName LevelName, bool bAbsolute = true, const FString& Options = FString());

	UFUNCTION(BlueprintCallable, Category = "Loading")
	void OpenLevelAsync(FName LevelName, bool bAbsolute = true, const FString& Options = TEXT(""));

	UFUNCTION(BlueprintPure, Category = "Loading")
	FORCEINLINE bool IsLoading() const { return bLoading; }

private:
	UPROPERTY()
	TObjectPtr<UTexture2D> LoadingImage;

	UPROPERTY()
	FSlateBrush LoadingBrush;

	UPROPERTY()
	TObjectPtr<ULoadingScreenWidget> LoadingWidget;

	FName PendingLevel;
	bool bPendingAbsolute = true;
	FString PendingOptions;
	bool bLoading = false;

	FDelegateHandle PrepareLoadingScreenHandle;
	FDelegateHandle PostLoadMapHandle;
	FDelegateHandle TravelFailureHandle;

	void ShowLoadingWidget();
	void HideLoadingWidget();
	void Travel();
	/** The movie player asks for the screen on every map load (the game start too) when none is set up for it */
	void HandlePrepareLoadingScreen();
	void HandlePostLoadMap(UWorld* World);
	void HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& Error);
	UTexture2D* GetLoadingImage();
};
