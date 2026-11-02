// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skills/SkillUpgradeWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/AssetManager.h"
#include "Engine/Texture2D.h"
#include "Utils/InputDeviceUtils.h"
#include "Framework/Application/SlateApplication.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/AudioComponent.h"


USkillUpgradeWidget::USkillUpgradeWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bSelected = false;
    bFocused = false;
	ProgressState = EStageProgress::None;
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
    // The focus given by a mouse hover doesn't select the skill (a click does), any other one is the keyboard or gamepad navigation.
    // (Not a hover check: CommonUI moves the cursor onto the gamepad focused widget, so it's hovered too)
    const bool bFocusedByHover = bHoverFocus;
    bHoverFocus = false;
    if (!IsValid(ActionButton)) return;

    // Hover sound for the keyboard & gamepad navigation (mouse hover plays it on its own)
    bool bNavigated = bFocused && !bFocusedByHover;
    if (bNavigated && IsValid(OnHoverActionSound))
    {
        constexpr float PitchMultiplier = 1;
        constexpr float SoundVolume = 1;
	    UGameplayStatics::SpawnSound2D(GetWorld(), OnHoverActionSound, SoundVolume, PitchMultiplier);
    }

    // Moving over the skills with the gamepad (d-pad, left stick) or arrows shows their details right away
    if (bNavigated) SetIsSelected(true);
}

FReply USkillUpgradeWidget::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
    // The skill button takes the focus over, so the accept action (A / Enter) clicks it
    TSharedPtr<SWidget> ButtonWidget = IsValid(ActionButton) ? ActionButton->GetCachedWidget() : nullptr;
    if (ButtonWidget.IsValid()) return FReply::Handled().SetUserFocus(ButtonWidget.ToSharedRef(), InFocusEvent.GetCause());
    return Super::NativeOnFocusReceived(InGeometry, InFocusEvent);
}

void USkillUpgradeWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (IsValid(ActionButton))
    {
        if (FSlateApplication::IsInitialized()) HoverCursorPosition = FSlateApplication::Get().GetCursorPos();
        // Skill details are shown in the description panel, the engine default text tooltip isn't needed
        ActionButton->SetToolTipText(FText::GetEmpty());
        ActionButton->OnClicked.AddDynamic(this, &USkillUpgradeWidget::OnActionClick);
        ActionButton->OnHovered.AddDynamic(this, &USkillUpgradeWidget::OnActionHover);
    }
}

void USkillUpgradeWidget::NativeDestruct()
{
    OnSkillProgressStateChanged.Clear();
    OnSkillSelected.Clear();
    OnUpgradeRequested.Clear();

    Super::NativeDestruct();
}

void USkillUpgradeWidget::SetIsSelected(bool bState)
{
    if (bSelected == bState) return;
    bSelected = bState;
    ApplyStateImage();
    OnSkillSelected.Broadcast(this, bSelected);
}

void USkillUpgradeWidget::LoadProgressState(EStageProgress State, bool bSelectedState)
{
    if (State == EStageProgress::None) return;
    // The same state is skipped unless another one is being loaded (it would override this one)
    bool bAlreadyApplied = ProgressState == State && LoadingState == EStageProgress::None;
    if (bAlreadyApplied || LoadingState == State) return;

    FSkillProgressionStyle* Style = ProgressStates.Find(State);
    if (!(Style && Style->IsImagesSet())) return;
    LoadingState = State;

    // Selection changed by the player while the images are loading is kept over the requested one
    const bool bSelectedOnRequest = bSelected;
    TWeakObjectPtr<USkillUpgradeWidget> WeakThis(this);
    UAssetManager::GetStreamableManager().RequestAsyncLoad(Style->GetImageLoadInfo(), [WeakThis, State, bSelectedState, bSelectedOnRequest]
    {
        USkillUpgradeWidget* Self = WeakThis.Get();
        if (!IsValid(Self) || Self->LoadingState != State) return;
        Self->LoadingState = EStageProgress::None;

        FSkillProgressionStyle* LoadedStyle = Self->ProgressStates.Find(State);
        if (!LoadedStyle) return;

        const EStageProgress PreviousState = Self->ProgressState;
        Self->ProgressState = State;
        if (Self->bSelected == bSelectedOnRequest) Self->bSelected = bSelectedState;
        Self->LoadedImages = { LoadedStyle->NormalImageState.Get(), LoadedStyle->SelectedImageState.Get() };
        Self->ApplyStateImage();

        Self->OnSkillProgressStateChanged.Broadcast(Self, Self->ProgressState, Self->bSelected, PreviousState);
        Self->OnSkillSelected.Broadcast(Self, Self->bSelected);
    });
}

void USkillUpgradeWidget::ApplyStateImage()
{
    FSkillProgressionStyle* Style = ProgressStates.Find(ProgressState);
    if (!Style) return;
    TSoftObjectPtr<UTexture2D>& Image = Style->GetImageByState(bSelected);
    // Images of the current state are held by LoadedImages, the synchronous load is only a safety net
    UTexture2D* Texture = Image.IsValid() ? Image.Get() : Image.LoadSynchronous();
    if (!IsValid(Texture)) return;
    LoadedImages.AddUnique(Texture);
    SetActionSkillImage(Texture);
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
    if (bSelected)
    {
        // Gamepad & keyboard select skills by moving over them, so pressing on the selected one upgrades it;
        // the mouse uses the description upgrade button
        const bool bGamepad = UInputDeviceUtils::IsGamepadUsed(GetOwningLocalPlayer());
        bool bPointerClick = IsValid(ActionButton) && ActionButton->IsHovered();
        if (bGamepad || !bPointerClick) OnUpgradeRequested.Broadcast(this);
        return;
    }

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
    bool bMouseMoved = false;
    if (FSlateApplication::IsInitialized())
    {
        const FVector2D CursorPosition = FSlateApplication::Get().GetCursorPos();
        bMouseMoved = !CursorPosition.Equals(HoverCursorPosition, 1.);
        HoverCursorPosition = CursorPosition;
    }
    // With the gamepad the (hidden) cursor follows the focus: the hover is the navigation itself, its sound is played on the focus
    if (UInputDeviceUtils::IsGamepadUsed(GetOwningLocalPlayer())) return;

    // Mouse hover moves the focus as well, so gamepad & keyboard navigation continues from it.
    // Only a real mouse move does it: the skill under a resting cursor (menu just opened) keeps the navigation focus where it is
    if (bMouseMoved && IsValid(ActionButton) && !ActionButton->HasAnyUserFocus())
    {
        bHoverFocus = true;
        ActionButton->SetFocus();
        bHoverFocus = ActionButton->HasAnyUserFocus();
    }

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
