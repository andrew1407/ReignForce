// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ControlsHint/ControlsHintWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"
#include "Engine/LocalPlayer.h"
#include "CommonInputSubsystem.h"

#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#include "Settings/ReignForceSettings.h"
#include "UI/Common/ReignForceGamepadControllerData.h"


namespace
{
    constexpr float GAMEPAD_ICON_SIZE = 34;
    constexpr float KEY_GAP = 6;
    constexpr float ROW_GAP = 5;
    /** Space between the labels & the keys columns */
    constexpr float LABEL_GAP = 22;

    FControlsHintEntry MakeEntry(const TCHAR* Label, const TCHAR* ActionName, int32 MaxKeys = 1)
    {
        FControlsHintEntry Entry;
        Entry.Label = FText::FromString(Label);
        Entry.ActionName = ActionName;
        Entry.MaxKeys = MaxKeys;
        return Entry;
    }

    /** Direction of a 2D axis key mapping (W, A, S, D): swizzled - vertical, negated - up / left; the others go last in their order */
    int32 GetDirectionOrder(const FEnhancedActionKeyMapping& Mapping)
    {
        bool bSwizzled = false;
        bool bNegated = false;
        for (const UInputModifier* Modifier : Mapping.Modifiers)
        {
            bSwizzled |= IsValid(Modifier) && Modifier->IsA<UInputModifierSwizzleAxis>();
            bNegated |= IsValid(Modifier) && Modifier->IsA<UInputModifierNegate>();
        }
        if (bSwizzled) return bNegated ? 2 : 0;
        return bNegated ? 1 : 3;
    }

    FSlateBrush MakeRoundedBrush(const FLinearColor& Fill, const FLinearColor& Outline, float Radius, float OutlineWidth)
    {
        FSlateBrush Brush;
        Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
        Brush.TintColor = Fill;
        Brush.OutlineSettings = FSlateBrushOutlineSettings(FVector4(Radius, Radius, Radius, Radius), FSlateColor(Outline), OutlineWidth);
        Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
        return Brush;
    }
}

UControlsHintWidget::UControlsHintWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    StartDelay = .6f;
    ShowDuration = 5;
    SlideDuration = .45f;
    DeviceSwitchFadeDuration = .3f;
    PanelMinWidth = 240;
    ScreenOffset = FVector2D(24, 24);

    // The modals palette (exit & upgrade confirmations): orange panel in a dark maroon frame, dark red buttons with light outlines
    PanelColor = FLinearColor(.495f, .2f, 0, .9f);
    OutlineColor = FLinearColor(.052f, 0, .006f, 1);
    TitleColor = FLinearColor::White;
    KeyCapColor = FLinearColor(.229f, .059f, .036f, 1);
    KeyCapOutlineColor = FLinearColor(.695f, .695f, .695f, 1);

    Entries = {
        MakeEntry(TEXT("Move"), TEXT("IA_Move"), 4),
        MakeEntry(TEXT("Aim"), TEXT("IA_Aim")),
        MakeEntry(TEXT("Attack"), TEXT("IA_Attack"), 2),
        MakeEntry(TEXT("Reload"), TEXT("IA_ReloadWeapon")),
        MakeEntry(TEXT("Start round"), TEXT("IA_ResetRoundState")),
        MakeEntry(TEXT("Skills menu"), TEXT("IA_ToggleSkillsMenu")),
        MakeEntry(TEXT("Save progress"), TEXT("IA_SaveGameProgress")),
        MakeEntry(TEXT("Pause"), TEXT("IA_TogglePause"), 2),
    };
}

TSharedRef<SWidget> UControlsHintWidget::RebuildWidget()
{
    if (!bBuilt) Build();
    return Super::RebuildWidget();
}

void UControlsHintWidget::Build()
{
    if (bBuilt || !IsValid(WidgetTree)) return;
    bBuilt = true;

    // Only a hint: never takes the clicks or the focus from the game
    SetVisibility(ESlateVisibility::HitTestInvisible);

    auto Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
    WidgetTree->RootWidget = Root;

    Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Panel"));
    Panel->SetBrush(MakeRoundedBrush(PanelColor, OutlineColor, 4, 4));
    Panel->SetPadding(FMargin(20, 12, 20, 14));
    if (UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(Panel))
    {
        PanelSlot->SetAnchors(FAnchors(1, 0));
        PanelSlot->SetAlignment(FVector2D(1, 0));
        PanelSlot->SetAutoSize(true);
        PanelSlot->SetPosition(FVector2D(-ScreenOffset.X, ScreenOffset.Y));
    }

    auto Frame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Frame"));
    Frame->SetMinDesiredWidth(PanelMinWidth);
    Panel->SetContent(Frame);
    auto Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
    Frame->SetContent(Column);

    auto Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
    Title->SetText(FText::FromString(TEXT("Controls")));
    Title->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 22));
    Title->SetColorAndOpacity(TitleColor);
    Title->SetShadowOffset(FVector2D(1));
    Title->SetShadowColorAndOpacity(FLinearColor(0, 0, 0, .6f));
    Column->AddChildToVerticalBox(Title);

    // Thin frame colored line under the title
    auto Separator = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Separator"));
    FSlateBrush SeparatorBrush;
    SeparatorBrush.ImageSize = FVector2D(1, 2);
    Separator->SetBrush(SeparatorBrush);
    Separator->SetColorAndOpacity(OutlineColor * FLinearColor(1, 1, 1, .8f));
    if (UVerticalBoxSlot* SeparatorSlot = Column->AddChildToVerticalBox(Separator))
    {
        SeparatorSlot->SetHorizontalAlignment(HAlign_Fill);
        SeparatorSlot->SetPadding(FMargin(0, 6, 0, 8));
    }

    // Two columns sized by their content: the labels & right after them the keys (no stretched gap between them)
    Table = WidgetTree->ConstructWidget<UGridPanel>(UGridPanel::StaticClass(), TEXT("Table"));
    Column->AddChildToVerticalBox(Table);

    FillTable(IsGamepadUsed());
    // Off screen (right side) until it slides in
    ApplySlide(0);
}

void UControlsHintWidget::ShowInputDevice(bool bGamepad)
{
    bDeviceSwitchPending = false;
    TableOpacity = 1;
    if (IsValid(Table)) Table->SetRenderOpacity(TableOpacity);
    FillTable(bGamepad);
}

void UControlsHintWidget::FillTable(bool bGamepad)
{
    bShowingGamepad = bGamepad;
    if (!(IsValid(Table) && IsValid(WidgetTree))) return;
    Table->ClearChildren();

    const UInputMappingContext* Context = UReignForceSettings::Get()->RemappableInputContext.LoadSynchronous();
    const FSlateFontInfo LabelFont = FCoreStyle::GetDefaultFontStyle("Bold", 17);
    const FSlateFontInfo KeyFont = FCoreStyle::GetDefaultFontStyle("Bold", 14);

    // The keys column takes the space the minimum panel width adds, the keys stay next to the labels
    Table->SetColumnFill(0, 0);
    Table->SetColumnFill(1, 1);

    int32 RowIndex = 0;
    for (const FControlsHintEntry& Entry : Entries)
    {
        const TArray<FKey> Keys = FindActionKeys(Context, Entry.ActionName, bGamepad, Entry.MaxKeys, GetOwningLocalPlayer());
        if (Keys.IsEmpty()) continue;

        auto Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        Label->SetText(Entry.Label);
        Label->SetFont(LabelFont);
        Label->SetColorAndOpacity(FLinearColor::White);
        Label->SetShadowOffset(FVector2D(1));
        Label->SetShadowColorAndOpacity(FLinearColor(0, 0, 0, .6f));
        if (UGridSlot* LabelSlot = Table->AddChildToGrid(Label, RowIndex, 0))
        {
            LabelSlot->SetVerticalAlignment(VAlign_Center);
            LabelSlot->SetPadding(FMargin(0, ROW_GAP * .5f, LABEL_GAP, ROW_GAP * .5f));
        }

        auto KeysRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
        for (int32 i = 0; i < Keys.Num(); ++i)
        {
            UWidget* KeyWidget = bGamepad ? MakeGamepadIcon(Keys[i], KeyFont) : MakeKeyCap(Keys[i], KeyFont);
            if (UHorizontalBoxSlot* KeySlot = KeysRow->AddChildToHorizontalBox(KeyWidget))
            {
                KeySlot->SetVerticalAlignment(VAlign_Center);
                KeySlot->SetPadding(FMargin(i > 0 ? KEY_GAP : 0, 0, 0, 0));
            }
        }
        // Rows are as tall as the gamepad icons, so the table keeps its height when the device changes
        auto KeysBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
        KeysBox->SetMinDesiredHeight(GAMEPAD_ICON_SIZE);
        KeysBox->SetContent(KeysRow);
        if (UGridSlot* KeysSlot = Table->AddChildToGrid(KeysBox, RowIndex, 1))
        {
            KeysSlot->SetVerticalAlignment(VAlign_Center);
            KeysSlot->SetHorizontalAlignment(HAlign_Left);
            KeysSlot->SetPadding(FMargin(0, ROW_GAP * .5f));
        }
        ++RowIndex;
    }
}

UWidget* UControlsHintWidget::MakeKeyCap(const FKey& Key, const FSlateFontInfo& Font)
{
    auto Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    Text->SetText(GetKeyCapText(Key));
    Text->SetFont(Font);
    Text->SetColorAndOpacity(FLinearColor::White);
    Text->SetJustification(ETextJustify::Center);

    auto Cap = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
    Cap->SetBrush(MakeRoundedBrush(KeyCapColor, KeyCapOutlineColor, 3, 1.5f));
    Cap->SetPadding(FMargin(8, 3));
    Cap->SetHorizontalAlignment(HAlign_Center);
    Cap->SetVerticalAlignment(VAlign_Center);
    Cap->SetContent(Text);

    // Single letters are square caps, longer names (LMB, Space) are wider
    auto Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
    Size->SetMinDesiredWidth(30);
    Size->SetMinDesiredHeight(28);
    Size->SetContent(Cap);
    return Size;
}

UWidget* UControlsHintWidget::MakeGamepadIcon(const FKey& Key, const FSlateFontInfo& Font)
{
    FSlateBrush Brush;
    if (!UReignForceGamepadControllerData::GetGamepadKeyBrush(Key, Brush)) return MakeKeyCap(Key, Font);
    Brush.ImageSize = FVector2D(GAMEPAD_ICON_SIZE);
    auto Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
    Icon->SetBrush(Brush);
    return Icon;
}

TArray<FKey> UControlsHintWidget::FindActionKeys(const UInputMappingContext* Context, FName ActionName, bool bGamepad, int32 MaxKeys, const ULocalPlayer* Player)
{
    if (!IsValid(Context)) return {};

    UEnhancedInputLocalPlayerSubsystem* Subsystem = Player ? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Player) : nullptr;
    const UEnhancedInputUserSettings* Settings = IsValid(Subsystem) ? Subsystem->GetUserSettings() : nullptr;
    const UEnhancedPlayerMappableKeyProfile* Profile = IsValid(Settings) ? Settings->GetActiveKeyProfile() : nullptr;

    TArray<TPair<int32, FKey>> Found;
    for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
    {
        if (!(IsValid(Mapping.Action) && Mapping.Action->GetFName() == ActionName)) continue;
        if (Mapping.Key.IsTouch() || Mapping.Key.IsGamepadKey() != bGamepad) continue;

        FKey Key = Mapping.Key;
        // Keyboard & mouse keys may be rebound by the player (controls settings)
        const FKeyMappingRow* PlayerRow = Profile && Mapping.IsPlayerMappable() ? Profile->FindKeyMappingRow(Mapping.GetMappingName()) : nullptr;
        if (PlayerRow)
        {
            for (const FPlayerKeyMapping& PlayerMapping : PlayerRow->Mappings)
            {
                if (PlayerMapping.GetDefaultKey() != Mapping.Key) continue;
                Key = PlayerMapping.GetCurrentKey();
                break;
            }
        }
        if (!Key.IsValid() || Found.ContainsByPredicate([&] (const TPair<int32, FKey>& Item) { return Item.Value == Key; })) continue;
        Found.Emplace(GetDirectionOrder(Mapping), Key);
    }

    // Movement keys read as W A S D, the other actions keep the mapping context order
    Found.StableSort([] (const TPair<int32, FKey>& A, const TPair<int32, FKey>& B) { return A.Key < B.Key; });
    TArray<FKey> Keys;
    for (int32 i = 0; i < Found.Num() && i < MaxKeys; ++i) Keys.Add(Found[i].Value);
    return Keys;
}

FText UControlsHintWidget::GetKeyCapText(const FKey& Key)
{
    if (Key == EKeys::LeftMouseButton) return FText::FromString(TEXT("LMB"));
    if (Key == EKeys::RightMouseButton) return FText::FromString(TEXT("RMB"));
    if (Key == EKeys::MiddleMouseButton) return FText::FromString(TEXT("MMB"));
    if (Key == EKeys::Escape) return FText::FromString(TEXT("Esc"));
    if (Key == EKeys::SpaceBar) return FText::FromString(TEXT("Space"));
    if (Key == EKeys::LeftControl || Key == EKeys::RightControl) return FText::FromString(TEXT("Ctrl"));
    if (Key == EKeys::LeftShift || Key == EKeys::RightShift) return FText::FromString(TEXT("Shift"));
    if (Key == EKeys::LeftAlt || Key == EKeys::RightAlt) return FText::FromString(TEXT("Alt"));
    constexpr bool bLongDisplayName = false;
    return Key.GetDisplayName(bLongDisplayName);
}

void UControlsHintWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    Advance(InDeltaTime);
}

void UControlsHintWidget::Advance(float InDeltaTime)
{
    if (Phase == EControlsHintPhase::Finished) return;

    // The table follows the used device: it fades out, gets the other keys & fades in
    const bool bGamepad = IsGamepadUsed();
    const bool bTargetGamepad = bDeviceSwitchPending ? bPendingGamepad : bShowingGamepad;
    if (bGamepad != bTargetGamepad)
    {
        bPendingGamepad = bGamepad;
        bDeviceSwitchPending = bGamepad != bShowingGamepad;
    }
    const float FadeSpeed = 2.f / DeviceSwitchFadeDuration;
    if (bDeviceSwitchPending)
    {
        TableOpacity = Phase == EControlsHintPhase::Waiting ? 0.f : FMath::Max(0.f, TableOpacity - InDeltaTime * FadeSpeed);
        if (TableOpacity <= 0)
        {
            bDeviceSwitchPending = false;
            FillTable(bPendingGamepad);
        }
    }
    else
    {
        TableOpacity = FMath::Min(1.f, TableOpacity + InDeltaTime * FadeSpeed);
    }
    if (IsValid(Table) && !FMath::IsNearlyEqual(Table->GetRenderOpacity(), TableOpacity)) Table->SetRenderOpacity(TableOpacity);

    // Waiting & showing count the game time (stopped while paused), the slides run on their own
    const bool bPaused = IsGamePaused();
    switch (Phase)
    {
        case EControlsHintPhase::Waiting:
            if (!bPaused) PhaseTime += InDeltaTime;
            if (PhaseTime < StartDelay) break;
            Phase = EControlsHintPhase::SlidingIn;
            PhaseTime = 0;
            break;

        case EControlsHintPhase::SlidingIn:
            PhaseTime += InDeltaTime;
            ApplySlide(FMath::InterpEaseOut(0.f, 1.f, FMath::Min(1.f, PhaseTime / SlideDuration), 3.f));
            if (PhaseTime < SlideDuration) break;
            Phase = EControlsHintPhase::Shown;
            PhaseTime = 0;
            break;

        case EControlsHintPhase::Shown:
            if (!bPaused) PhaseTime += InDeltaTime;
            if (PhaseTime < ShowDuration) break;
            Phase = EControlsHintPhase::SlidingOut;
            PhaseTime = 0;
            break;

        case EControlsHintPhase::SlidingOut:
            PhaseTime += InDeltaTime;
            ApplySlide(1 - FMath::InterpEaseIn(0.f, 1.f, FMath::Min(1.f, PhaseTime / SlideDuration), 3.f));
            if (PhaseTime < SlideDuration) break;
            Phase = EControlsHintPhase::Finished;
            RemoveFromParent();
            break;

        default:
            break;
    }
}

void UControlsHintWidget::ApplySlide(float Shown)
{
    if (!IsValid(Panel)) return;
    // Fully hidden past the screen right edge, shown at its place
    const float PanelTotalWidth = FMath::Max(Panel->GetDesiredSize().X, PanelMinWidth + 40);
    const float Offset = (1 - Shown) * (PanelTotalWidth + ScreenOffset.X + 16);
    Panel->SetRenderTranslation(FVector2D(Offset, 0));
    Panel->SetRenderOpacity(FMath::Clamp(Shown * 1.5f, 0.f, 1.f));
}

bool UControlsHintWidget::IsGamepadUsed() const
{
    const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
    return InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;
}

bool UControlsHintWidget::IsGamePaused() const
{
    return UGameplayStatics::IsGamePaused(this);
}
