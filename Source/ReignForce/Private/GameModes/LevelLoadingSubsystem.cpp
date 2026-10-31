// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/LevelLoadingSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Misc/PackageName.h"
#include "MoviePlayer.h"

#include "Styling/CoreStyle.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#include "CommonActivatableWidget.h"
#include "Settings/ReignForceSettings.h"
#include "UI/Loading/LoadingScreenWidget.h"


namespace
{
    constexpr int32 LOADING_SCREEN_Z_ORDER = 1000;
}

ULevelLoadingSubsystem* ULevelLoadingSubsystem::Get(const UObject* WorldContextObject)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    UGameInstance* GameInstance = IsValid(World) ? World->GetGameInstance() : nullptr;
    return IsValid(GameInstance) ? GameInstance->GetSubsystem<ULevelLoadingSubsystem>() : nullptr;
}

void ULevelLoadingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    PreLoadMapHandle = FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &ULevelLoadingSubsystem::HandlePreLoadMap);
    PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ULevelLoadingSubsystem::HandlePostLoadMap);
}

void ULevelLoadingSubsystem::Deinitialize()
{
    FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadMapHandle);
    FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);

    Super::Deinitialize();
}

void ULevelLoadingSubsystem::OpenLevel(const UObject* WorldContextObject, FName LevelName, bool bAbsolute, const FString& Options)
{
    ULevelLoadingSubsystem* Subsystem = Get(WorldContextObject);
    if (IsValid(Subsystem)) return Subsystem->OpenLevelAsync(LevelName, bAbsolute, Options);
    UGameplayStatics::OpenLevel(WorldContextObject, LevelName, bAbsolute, Options);
}

void ULevelLoadingSubsystem::OpenLevelAsync(FName LevelName, bool bAbsolute, const FString& Options)
{
    if (bLoading || LevelName.IsNone()) return;
    bLoading = true;
    PendingLevel = LevelName;
    bPendingAbsolute = bAbsolute;
    PendingOptions = Options;

    ShowLoadingWidget();

    const FString Package = ResolveLevelPackage(LevelName);
    if (Package.IsEmpty()) return Travel();
    LoadPackageAsync(Package, FLoadPackageAsyncDelegate::CreateUObject(this, &ULevelLoadingSubsystem::HandlePackageLoaded));
}

void ULevelLoadingSubsystem::ShowLoadingWidget()
{
    UGameInstance* GameInstance = GetGameInstance();
    UWorld* World = IsValid(GameInstance) ? GameInstance->GetWorld() : nullptr;
    if (!IsValid(World)) return;

    // Menus underneath stop handling the input (back actions, focus) while the level is loading
    TArray<UUserWidget*> Menus;
    constexpr bool bTopLevelOnly = true;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, Menus, UCommonActivatableWidget::StaticClass(), bTopLevelOnly);
    for (UUserWidget* Menu : Menus)
        if (auto Activatable = Cast<UCommonActivatableWidget>(Menu)) Activatable->DeactivateWidget();

    LoadingWidget = CreateWidget<ULoadingScreenWidget>(GameInstance, ULoadingScreenWidget::StaticClass());
    if (!IsValid(LoadingWidget)) return;
    LoadingWidget->SetImage(GetLoadingImage());
    LoadingWidget->AddToViewport(LOADING_SCREEN_Z_ORDER);
}

void ULevelLoadingSubsystem::Travel()
{
    UGameInstance* GameInstance = GetGameInstance();
    UWorld* World = IsValid(GameInstance) ? GameInstance->GetWorld() : nullptr;
    if (!IsValid(World))
    {
        bLoading = false;
        return;
    }
    UGameplayStatics::OpenLevel(World, PendingLevel, bPendingAbsolute, PendingOptions);
}

void ULevelLoadingSubsystem::HandlePackageLoaded(const FName& PackageName, UPackage* Package, EAsyncLoadingResult::Type Result)
{
    if (Result == EAsyncLoadingResult::Succeeded) PreloadedLevel = Package;
    // A failed preload is not fatal, the travel loads the level on its own
    Travel();
}

void ULevelLoadingSubsystem::HandlePreLoadMap(const FString& MapName)
{
    if (IsRunningDedicatedServer() || !IsMoviePlayerEnabled() || !GetMoviePlayer()) return;

    UTexture2D* Image = GetLoadingImage();
    const UReignForceSettings* Settings = UReignForceSettings::Get();

    FLoadingScreenAttributes LoadingScreen;
    LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
    LoadingScreen.MinimumLoadingScreenDisplayTime = Settings ? Settings->MinLoadingScreenDisplayTime : 0;
    LoadingScreen.WidgetLoadingScreen = SNew(SOverlay)
        + SOverlay::Slot()
        [
            SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("BlackBrush"))
        ]
        + SOverlay::Slot()
        [
            SNew(SScaleBox)
            .Stretch(EStretch::ScaleToFill)
            [
                SNew(SImage).Image(IsValid(Image) ? &LoadingBrush : nullptr)
            ]
        ]
        + SOverlay::Slot()
        .HAlign(HAlign_Right)
        .VAlign(VAlign_Bottom)
        .Padding(48)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [
                SNew(SCircularThrobber).Radius(18)
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(16, 0, 0, 0)
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Loading...")))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
                .ShadowOffset(FVector2D(2))
                .ShadowColorAndOpacity(FLinearColor(0, 0, 0, .8f))
            ]
        ];
    GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
}

void ULevelLoadingSubsystem::HandlePostLoadMap(UWorld* World)
{
    PreloadedLevel = nullptr;
    LoadingWidget = nullptr;
    bLoading = false;
}

FString ULevelLoadingSubsystem::ResolveLevelPackage(FName LevelName) const
{
    const FString Name = LevelName.ToString();
    if (FPackageName::IsValidLongPackageName(Name)) return Name;

    IAssetRegistry* AssetRegistry = IAssetRegistry::Get();
    if (!AssetRegistry) return FString();
    TArray<FAssetData> Levels;
    AssetRegistry->GetAssetsByClass(UWorld::StaticClass()->GetClassPathName(), Levels);
    const FAssetData* Level = Levels.FindByPredicate([&] (const FAssetData& Asset) { return Asset.AssetName == LevelName; });
    return Level ? Level->PackageName.ToString() : FString();
}

UTexture2D* ULevelLoadingSubsystem::GetLoadingImage()
{
    if (IsValid(LoadingImage)) return LoadingImage;
    const UReignForceSettings* Settings = UReignForceSettings::Get();
    LoadingImage = Settings ? Settings->LoadingScreenImage.LoadSynchronous() : nullptr;
    if (!IsValid(LoadingImage)) return nullptr;

    LoadingBrush.SetResourceObject(LoadingImage);
    LoadingBrush.ImageSize = FVector2D(LoadingImage->GetSizeX(), LoadingImage->GetSizeY());
    return LoadingImage;
}
