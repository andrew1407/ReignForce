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
    bFocused = false;
	ProgressState = EStageProgress::None;
    FocusedScale = 1.18f;
    FocusedTint = FLinearColor(1.3f, 1.3f, 1.3f, 1);
    SetIsFocusable(true);
}

void USkillUpgradeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    bool bHasFocus = HasAnyUserFocus() || (IsValid(ActionButton) && ActionButton->HasAnyUserFocus());
    if (bHasFocus != bFocused) SetFocusHighlight(bHasFocus);
}

void USkillUpgradeWidget::SetFocusHighlight(bool bState)
{
    bFocused = bState;
    if (!IsValid(ActionButton)) return;

    ActionButton->SetRenderTransformPivot(FVector2D(.5f));
    ActionButton->SetRenderScale(FVector2D(bFocused ? FocusedScale : 1));
    // Skill image is the button brush, so the background tint brightens it
    ActionButton->SetBackgroundColor(bFocused ? DefaultActionBackground * FocusedTint : DefaultActionBackground);

    // Hover sound for the keyboard & gamepad navigation (mouse hover plays it on its own)
    bool bNavigated = bFocused && !ActionButton->IsHovered();
    if (bNavigated && IsValid(OnHoverActionSound))
    {
        constexpr float PitchMultiplier = 1;
        constexpr float SoundVolume = 1;
	    UGameplayStatics::SpawnSound2D(GetWorld(), OnHoverActionSound, SoundVolume, PitchMultiplier);
    }
}

void USkillUpgradeWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (IsValid(ActionButton))
    {
        DefaultActionBackground = ActionButton->GetBackgroundColor();
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
    // Mouse hover moves the focus as well, so gamepad & keyboard navigation continues from it
    if (IsValid(ActionButton) && !ActionButton->HasAnyUserFocus()) ActionButton->SetFocus();

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
