// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/LevelLoadingSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/GameModeBase.h"
#include "Containers/Ticker.h"
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
#include "GameModes/Components/BackgroundMusicComponent.h"
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

    // The movie player handles the map loading start before any other listener, so the screen is given to it on its request
    // (a screen set up on PreLoadMap would be shown only on the next map change)
    if (!IsRunningDedicatedServer() && IsMoviePlayerEnabled() && GetMoviePlayer())
        PrepareLoadingScreenHandle = GetMoviePlayer()->OnPrepareLoadingScreen().AddUObject(this, &ULevelLoadingSubsystem::HandlePrepareLoadingScreen);
    PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ULevelLoadingSubsystem::HandlePostLoadMap);
    if (GEngine) TravelFailureHandle = GEngine->OnTravelFailure().AddUObject(this, &ULevelLoadingSubsystem::HandleTravelFailure);
}

void ULevelLoadingSubsystem::Deinitialize()
{
    if (PrepareLoadingScreenHandle.IsValid() && GetMoviePlayer())
        GetMoviePlayer()->OnPrepareLoadingScreen().Remove(PrepareLoadingScreenHandle);
    FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
    if (GEngine) GEngine->OnTravelFailure().Remove(TravelFailureHandle);
    if (TravelTickerHandle.IsValid()) FTSTicker::GetCoreTicker().RemoveTicker(TravelTickerHandle);
    TravelTickerHandle.Reset();

    Super::Deinitialize();
}

bool ULevelLoadingSubsystem::IsLevelRestarted(const UObject* WorldContextObject)
{
    const ULevelLoadingSubsystem* Subsystem = Get(WorldContextObject);
    return IsValid(Subsystem) && Subsystem->IsLevelRestarted();
}

void ULevelLoadingSubsystem::OpenLevel(const UObject* WorldContextObject, FName LevelName, bool bAbsolute, const FString& Options, bool bRestart)
{
    ULevelLoadingSubsystem* Subsystem = Get(WorldContextObject);
    if (IsValid(Subsystem)) return Subsystem->OpenLevelAsync(LevelName, bAbsolute, Options, bRestart);
    UGameplayStatics::OpenLevel(WorldContextObject, LevelName, bAbsolute, Options);
}

void ULevelLoadingSubsystem::OpenLevelAsync(FName LevelName, bool bAbsolute, const FString& Options, bool bRestart)
{
    if (bLoading || LevelName.IsNone()) return;
    bLoading = true;
    bShowImage = !bRestart;
    // Remembered for the new level (its HUD checks it on the begin play), the previous one is back if the travel fails
    bPreviousLevelRestarted = bLevelRestarted;
    bLevelRestarted = bRestart;
    PendingLevel = LevelName;
    bPendingAbsolute = bAbsolute;
    PendingOptions = Options;

    StopMusic();
    ShowLoadingWidget();

    // The loading screen is drawn first, then the movie player keeps it on screen while the map loads.
    // The map package isn't preloaded: only the package would stay referenced, its world was collected
    // before the travel and the travel failed with "Failed to load package".
    // The core ticker is used, not a world timer: those don't tick while the game is paused, so a level opened
    // from the pause menu (restart, go to menu) never travelled and the loading screen stayed forever
    if (TravelTickerHandle.IsValid()) FTSTicker::GetCoreTicker().RemoveTicker(TravelTickerHandle);
    TravelTickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this] (float)
    {
        TravelTickerHandle.Reset();
        Travel();
        return false;
    }));
}

void ULevelLoadingSubsystem::StopMusic()
{
    UGameInstance* GameInstance = GetGameInstance();
    UWorld* World = IsValid(GameInstance) ? GameInstance->GetWorld() : nullptr;
    AGameModeBase* GameMode = IsValid(World) ? World->GetAuthGameMode() : nullptr;
    UBackgroundMusicComponent* Music = IsValid(GameMode) ? GameMode->FindComponentByClass<UBackgroundMusicComponent>() : nullptr;
    // No music under the loading screen: the playing tracks are stopped & the ones still being loaded won't start
    if (IsValid(Music)) Music->StopForLevelTravel();
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
    // No image - the widget stays black
    LoadingWidget->SetImage(bShowImage ? GetLoadingImage() : nullptr);
    LoadingWidget->AddToViewport(LOADING_SCREEN_Z_ORDER);
}

void ULevelLoadingSubsystem::HideLoadingWidget()
{
    if (IsValid(LoadingWidget)) LoadingWidget->RemoveFromParent();
    LoadingWidget = nullptr;

    UGameInstance* GameInstance = GetGameInstance();
    UWorld* World = IsValid(GameInstance) ? GameInstance->GetWorld() : nullptr;
    if (!IsValid(World)) return;

    // The level stays the same, so its menus get the input back
    TArray<UUserWidget*> Menus;
    constexpr bool bTopLevelOnly = true;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, Menus, UCommonActivatableWidget::StaticClass(), bTopLevelOnly);
    for (UUserWidget* Menu : Menus)
    {
        auto Activatable = Cast<UCommonActivatableWidget>(Menu);
        if (IsValid(Activatable) && Activatable->IsInViewport() && !Activatable->IsActivated()) Activatable->ActivateWidget();
    }
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

void ULevelLoadingSubsystem::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& Error)
{
    UE_LOG(LogTemp, Warning, TEXT("Level '%s' opening failed: %s"), *PendingLevel.ToString(), *Error);
    bLoading = false;
    bShowImage = true;
    bLevelRestarted = bPreviousLevelRestarted;
    HideLoadingWidget();
}

void ULevelLoadingSubsystem::HandlePrepareLoadingScreen()
{
    if (!GetMoviePlayer()) return;

    // Plain black (only the loading indicator) for a level opened without the image
    UTexture2D* Image = bShowImage ? GetLoadingImage() : nullptr;
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
    LoadingWidget = nullptr;
    bLoading = false;
    // Other map loads (the game start) show the image again
    bShowImage = true;
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
