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
 * Opens levels asynchronously: the level package is streamed in behind a loading screen (main menu image),
 * then the travel happens with the same image shown by the movie player during the map switch
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

	/** Holds the streamed in level until the travel picks it up */
	UPROPERTY()
	TObjectPtr<UPackage> PreloadedLevel;

	FName PendingLevel;
	bool bPendingAbsolute = true;
	FString PendingOptions;
	bool bLoading = false;

	FDelegateHandle PreLoadMapHandle;
	FDelegateHandle PostLoadMapHandle;

	void ShowLoadingWidget();
	void Travel();
	void HandlePackageLoaded(const FName& PackageName, UPackage* Package, EAsyncLoadingResult::Type Result);
	void HandlePreLoadMap(const FString& MapName);
	void HandlePostLoadMap(UWorld* World);
	FString ResolveLevelPackage(FName LevelName) const;
	UTexture2D* GetLoadingImage();
};
