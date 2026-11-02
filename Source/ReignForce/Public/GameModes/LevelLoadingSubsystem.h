// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/UObjectGlobals.h"
#include "Styling/SlateBrush.h"
#include "Containers/Ticker.h"
#include "LevelLoadingSubsystem.generated.h"


class UTexture2D;
class ULoadingScreenWidget;

/**
 * Opens levels behind a loading screen (black & white main menu image): the map is loaded asynchronously,
 * then the movie player keeps the screen up while it's opened
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
	static void OpenLevel(const UObject* WorldContextObject, FName LevelName, bool bAbsolute = true, const FString& Options = FString(), bool bRestart = false);

	/**
	 * A restart (the same level again: pause menu restart, the countdown after a lost round) has no loading screen: the game stays
	 * on the screen with the input disabled until the level is reopened, and the restarted level doesn't show the opening hints (the controls table)
	 */
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void OpenLevelAsync(FName LevelName, bool bAbsolute = true, const FString& Options = TEXT(""), bool bRestart = false);

	/** The loading screen shows the black & white image (false while a level is restarted) */
	UFUNCTION(BlueprintPure, Category = "Loading")
	FORCEINLINE bool IsShowingImage() const { return bShowImage; }

	/** The current level was opened by a restart (kept until the next level opening) */
	UFUNCTION(BlueprintPure, Category = "Loading")
	FORCEINLINE bool IsLevelRestarted() const { return bLevelRestarted; }

	/** Whether the current level (in the world's game instance) was opened by a restart */
	static bool IsLevelRestarted(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Loading")
	FORCEINLINE bool IsLoading() const { return bLoading; }

private:
	UPROPERTY()
	TObjectPtr<UTexture2D> LoadingImage;

	UPROPERTY()
	FSlateBrush LoadingBrush;

	UPROPERTY()
	TObjectPtr<ULoadingScreenWidget> LoadingWidget;

	/** Held until it's opened */
	UPROPERTY()
	TObjectPtr<UWorld> PreloadedWorld;

	FName PendingLevel;
	bool bPendingAbsolute = true;
	FString PendingOptions;
	bool bLoading = false;
	/** Kept until the map is loaded: the movie player asks for its screen in the middle of the travel */
	bool bShowImage = true;
	bool bLevelRestarted = false;
	bool bPreviousLevelRestarted = false;

	FDelegateHandle PrepareLoadingScreenHandle;
	FDelegateHandle PostLoadMapHandle;
	FDelegateHandle TravelFailureHandle;
	FTSTicker::FDelegateHandle TravelTickerHandle;

	UWorld* GetCurrentWorld() const;
	void StopMusic();
	/** Menus of the current world stop (or resume) handling the input */
	void SetMenusActive(bool bActive);
	/** The local player's gameplay input (movement, attacks, pause toggling) */
	void SetPlayerInputEnabled(bool bEnabled);
	void ShowLoadingWidget();
	void HideLoadingWidget();
	void PreloadLevel();
	/** Empty if the level is opened without the preload */
	FString FindPreloadablePackage() const;
	void HandleLevelPreloaded(const FName& PackageName, UPackage* Package, EAsyncLoadingResult::Type Result);
	void ScheduleTravel();
	void Travel();
	/** The movie player asks for the screen on every map load (the game start too) when none is set up for it */
	void HandlePrepareLoadingScreen();
	void HandlePostLoadMap(UWorld* World);
	void HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& Error);
	UTexture2D* GetLoadingImage();
};
