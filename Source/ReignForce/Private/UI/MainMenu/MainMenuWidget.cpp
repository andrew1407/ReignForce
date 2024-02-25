// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MainMenu/MainMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Button.h"


UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    MainGameLevelName = NAME_None;
}

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (IsValid(PlayButton))
    {
        PlayButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayClick);
        PlayButton->SetFocus();
    }

    if (IsValid(ExitButton)) ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnExitClick);
}

void UMainMenuWidget::OnPlayClick()
{
    if (MainGameLevelName.IsNone()) return;

    constexpr bool bAbsolute = true;
	UGameplayStatics::OpenLevel(GetWorld(), MainGameLevelName, bAbsolute);
}

void UMainMenuWidget::OnExitClick()
{
    // Specify the quit preference (e.g., Quit, Restart, etc.)
    TEnumAsByte<EQuitPreference::Type> QuitPreference = EQuitPreference::Quit;

    // Specify whether to ignore platform restrictions
    constexpr bool bIgnorePlatformRestrictions = false;

    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), QuitPreference, bIgnorePlatformRestrictions);
}
