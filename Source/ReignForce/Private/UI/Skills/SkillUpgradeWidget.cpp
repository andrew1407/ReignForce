// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skills/SkillUpgradeWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/AssetManager.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/AudioComponent.h"


USkillUpgradeWidget::USkillUpgradeWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bSelected = false;
	ProgressState = EStageProgress::None;
    SetIsFocusable(true);
}

void USkillUpgradeWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (IsValid(ActionButton))
    {
        ActionButton->OnClicked.AddDynamic(this, &USkillUpgradeWidget::OnActionClick);
        ActionButton->OnHovered.AddDynamic(this, &USkillUpgradeWidget::OnActionHover);
    }
}

void USkillUpgradeWidget::NativeDestruct()
{
    OnSkillProgressStateChanged.Clear();
    OnSkillSelected.Clear();

    Super::NativeDestruct();
}

void USkillUpgradeWidget::SetIsSelected(bool bState)
{
    if (bSelected == bState) return;
    bSelected = bState;

    FSkillProgressionStyle* Style = ProgressStates.Find(ProgressState);
    if (Style) SetActionSkillImage(Style->GetImageByState(bSelected).Get());

    OnSkillSelected.Broadcast(this, bSelected);
}

void USkillUpgradeWidget::LoadProgressState(EStageProgress State, bool bSelectedState)
{
    if (State == EStageProgress::None || ProgressState == State) return;
    FSkillProgressionStyle* Style = ProgressStates.Find(State);

    if (!(Style && Style->IsImagesSet())) return;

    UAssetManager::GetStreamableManager().RequestAsyncLoad(Style->GetImageLoadInfo(), [this, Style, State, bSelectedState]
    {
        if (!(Style && Style->IsImagesLoaded())) return;
    
        EStageProgress PreviousState = ProgressState;
        ProgressState = State;
        bSelected = bSelectedState;

        FSkillProgressionStyle* PreviousStyle = ProgressStates.Find(PreviousState);
        if (PreviousStyle) PreviousStyle->Clear();

        TSoftObjectPtr<UTexture2D>& Image = Style->GetImageByState(bSelected);
        SetActionSkillImage(Image.Get());

        OnSkillProgressStateChanged.Broadcast(this, ProgressState, bSelected, PreviousState);
        OnSkillSelected.Broadcast(this, bSelected);
    });
}

UTexture2D* USkillUpgradeWidget::GetBaseSkillIcon() const
{
    FSkillProgressionStyle const* Style = ProgressStates.Find(ProgressState);
    if (!Style) return nullptr;
    constexpr bool bDefaultMode = false;
    return Style->NormalImageState.Get();
}

void USkillUpgradeWidget::OnActionClick()
{
    if (bSelected) return;

    if (IsValid(OnClickOnActionSound))
    {
        constexpr float PitchMultiplier = 1;
        constexpr float SoundVolume = 1;
	    UGameplayStatics::SpawnSound2D(GetWorld(), OnClickOnActionSound, SoundVolume, PitchMultiplier);
    }

    SetIsSelected(true);
}

void USkillUpgradeWidget::OnActionHover()
{
    if (IsValid(OnHoverActionSound))
    {
        constexpr float PitchMultiplier = 1;
        constexpr float SoundVolume = 1;
	    UGameplayStatics::SpawnSound2D(GetWorld(), OnHoverActionSound, SoundVolume, PitchMultiplier);
    }
}

void USkillUpgradeWidget::SetActionSkillImage(UTexture2D* Image)
{
    if (!(IsValid(ActionButton) && IsValid(Image))) return;

    FSlateBrush Brush;
    Brush.SetResourceObject(Image);
    Brush.SetImageSize(FVector2D(Image->GetSizeX(), Image->GetSizeY()));

    FButtonStyle ButtonStyle;

    FSlateColor NormalColor = ButtonStyle.Normal.TintColor;
    ButtonStyle.Normal = Brush;
    ButtonStyle.Normal.TintColor = NormalColor;

    FSlateColor HoveredColor = ButtonStyle.Hovered.TintColor;
    ButtonStyle.Hovered = Brush;
    ButtonStyle.Hovered.TintColor = HoveredColor;

    FSlateColor PressedColor = ButtonStyle.Pressed.TintColor;
    ButtonStyle.Pressed = Brush;
    ButtonStyle.Pressed.TintColor = PressedColor;

    ActionButton->SetStyle(ButtonStyle);
}
