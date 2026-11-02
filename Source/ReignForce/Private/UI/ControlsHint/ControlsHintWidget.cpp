// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ControlsHint/ControlsHintWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Algo/Find.h"
#include "Styling/CoreStyle.h"
#include "Engine/LocalPlayer.h"

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
#include "UI/Common/MenuLayoutUtils.h"
#include "UI/Common/ReignForceGamepadControllerData.h"
#include "Utils/InputDeviceUtils.h"


using namespace MenuLayoutUtils;

namespace
{
    constexpr float GAMEPAD_ICON_SIZE = 34;
    constexpr float KEY_GAP = 6;
    constexpr float HINT_ROW_GAP = 5;
    /** Space between the labels & the keys columns */
    constexpr float LABEL_GAP = 22;
    constexpr int32 HINT_TITLE_FONT_SIZE = 22;
    constexpr int32 LABEL_FONT_SIZE = 17;
    constexpr int32 KEY_FONT_SIZE = 14;
    constexpr float SLIDE_EASE_EXPONENT = 3;
    /** Panel room past the screen edge when it's hidden */
    constexpr float HIDDEN_PANEL_MARGIN = 16;

    /** Compact key cap names (the other keys use their short display names); built on the first use, after the engine keys */
    const TMap<FKey, const TCHAR*>& GetKeyCapNames()
    {
        static const TMap<FKey, const TCHAR*> Names = {
            { EKeys::LeftMouseButton, TEXT("LMB") },
            { EKeys::RightMouseButton, TEXT("RMB") },
            { EKeys::MiddleMouseButton, TEXT("MMB") },
            { EKeys::Escape, TEXT("Esc") },
            { EKeys::SpaceBar, TEXT("Space") },
            { EKeys::LeftControl, TEXT("Ctrl") },
            { EKeys::RightControl, TEXT("Ctrl") },
            { EKeys::LeftShift, TEXT("Shift") },
            { EKeys::RightShift, TEXT("Shift") },
            { EKeys::LeftAlt, TEXT("Alt") },
            { EKeys::RightAlt, TEXT("Alt") },
        };
        return Names;
    }

    /** A timeline phase: how long it lasts (game time stops while paused), the next one & the panel slide during it */
    struct FControlsHintPhaseStep
    {
        float Duration;
        bool bGameTime;
        EControlsHintPhase Next;
        float (*Slide)(float Alpha);
    };

    float SlideIn(float Alpha) { return FMath::InterpEaseOut(0.f, 1.f, Alpha, SLIDE_EASE_EXPONENT); }
    float SlideOut(float Alpha) { return 1 - FMath::InterpEaseIn(0.f, 1.f, Alpha, SLIDE_EASE_EXPONENT); }

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
        auto HasModifier = [&] (UClass* ModifierClass)
        {
            return Mapping.Modifiers.ContainsByPredicate([ModifierClass] (const UInputModifier* Modifier) { return IsValid(Modifier) && Modifier->IsA(ModifierClass); });
        };
        const bool bNegated = HasModifier(UInputModifierNegate::StaticClass());
        if (HasModifier(UInputModifierSwizzleAxis::StaticClass())) return bNegated ? 2 : 0;
        return bNegated ? 1 : 3;
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
        PanelSlot->SetAnchors(FAnchors(0, 0));
        PanelSlot->SetAlignment(FVector2D(0, 0));
        PanelSlot->SetAutoSize(true);
        PanelSlot->SetPosition(ScreenOffset);
    }

    auto Frame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Frame"));
    Frame->SetMinDesiredWidth(PanelMinWidth);
    Panel->SetContent(Frame);
    auto Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
    Frame->SetContent(Column);

    const FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", HINT_TITLE_FONT_SIZE);
    Column->AddChildToVerticalBox(MakeText(WidgetTree, FText::FromString(TEXT("Controls")), TitleFont, TitleColor, UTextBlock::StaticClass()));

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

    FillTable(UInputDeviceUtils::IsGamepadUsed(GetOwningLocalPlayer()));
    // Off screen (left side) until it slides in
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
    const FSlateFontInfo LabelFont = FCoreStyle::GetDefaultFontStyle("Bold", LABEL_FONT_SIZE);
    const FSlateFontInfo KeyFont = FCoreStyle::GetDefaultFontStyle("Bold", KEY_FONT_SIZE);

    // The keys column takes the space the minimum panel width adds, the keys stay next to the labels
    Table->SetColumnFill(0, 0);
    Table->SetColumnFill(1, 1);

    int32 RowIndex = 0;
    for (const FControlsHintEntry& Entry : Entries)
    {
        const TArray<FKey> Keys = FindActionKeys(Context, Entry.ActionName, bGamepad, Entry.MaxKeys, GetOwningLocalPlayer());
        if (Keys.IsEmpty()) continue;

        UTextBlock* Label = MakeText(WidgetTree, Entry.Label, LabelFont, FLinearColor::White, UTextBlock::StaticClass());
        if (UGridSlot* LabelSlot = Table->AddChildToGrid(Label, RowIndex, 0))
        {
            LabelSlot->SetVerticalAlignment(VAlign_Center);
            LabelSlot->SetPadding(FMargin(0, HINT_ROW_GAP * .5f, LABEL_GAP, HINT_ROW_GAP * .5f));
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
            KeysSlot->SetPadding(FMargin(0, HINT_ROW_GAP * .5f));
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

        // Keyboard & mouse keys may be rebound by the player (controls settings)
        const FKeyMappingRow* PlayerRow = Profile && Mapping.IsPlayerMappable() ? Profile->FindKeyMappingRow(Mapping.GetMappingName()) : nullptr;
        const FPlayerKeyMapping* PlayerMapping = PlayerRow
            ? Algo::FindByPredicate(PlayerRow->Mappings, [&] (const FPlayerKeyMapping& Item) { return Item.GetDefaultKey() == Mapping.Key; })
            : nullptr;
        const FKey Key = PlayerMapping ? PlayerMapping->GetCurrentKey() : Mapping.Key;
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
    if (const TCHAR* const* Name = GetKeyCapNames().Find(Key)) return FText::FromString(*Name);
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
    if (Phase == EControlsHintPhase::FINISHED) return;
    AdvanceDeviceSwitch(InDeltaTime);
    AdvancePhase(InDeltaTime);
}

void UControlsHintWidget::AdvanceDeviceSwitch(float DeltaTime)
{
    // The table follows the used device: it fades out, gets the other keys & fades in
    const bool bGamepad = UInputDeviceUtils::IsGamepadUsed(GetOwningLocalPlayer());
    const bool bTargetGamepad = bDeviceSwitchPending ? bPendingGamepad : bShowingGamepad;
    if (bGamepad != bTargetGamepad)
    {
        bPendingGamepad = bGamepad;
        bDeviceSwitchPending = bGamepad != bShowingGamepad;
    }

    const float FadeStep = DeltaTime * 2.f / DeviceSwitchFadeDuration;
    if (!bDeviceSwitchPending) TableOpacity = FMath::Min(1.f, TableOpacity + FadeStep);
    else TableOpacity = Phase == EControlsHintPhase::WAITING ? 0.f : FMath::Max(0.f, TableOpacity - FadeStep);
    if (bDeviceSwitchPending && TableOpacity <= 0)
    {
        bDeviceSwitchPending = false;
        FillTable(bPendingGamepad);
    }
    if (IsValid(Table) && !FMath::IsNearlyEqual(Table->GetRenderOpacity(), TableOpacity)) Table->SetRenderOpacity(TableOpacity);
}

void UControlsHintWidget::AdvancePhase(float DeltaTime)
{
    // Waiting & showing count the game time (stopped while paused), the slides run on their own
    const FControlsHintPhaseStep Steps[] = {
        { StartDelay, true, EControlsHintPhase::SLIDING_IN, nullptr },
        { SlideDuration, false, EControlsHintPhase::SHOWN, &SlideIn },
        { ShowDuration, true, EControlsHintPhase::SLIDING_OUT, nullptr },
        { SlideDuration, false, EControlsHintPhase::FINISHED, &SlideOut },
    };
    const FControlsHintPhaseStep& Step = Steps[static_cast<int32>(Phase)];

    if (!(Step.bGameTime && IsGamePaused())) PhaseTime += DeltaTime;
    if (Step.Slide) ApplySlide(Step.Slide(FMath::Min(1.f, PhaseTime / Step.Duration)));
    if (PhaseTime < Step.Duration) return;

    Phase = Step.Next;
    PhaseTime = 0;
    if (Phase == EControlsHintPhase::FINISHED) RemoveFromParent();
}

void UControlsHintWidget::ApplySlide(float Shown)
{
    if (!IsValid(Panel)) return;
    // Fully hidden past the screen left edge, shown at its place
    const float PanelTotalWidth = FMath::Max(Panel->GetDesiredSize().X, PanelMinWidth + 40);
    const float Offset = -(1 - Shown) * (PanelTotalWidth + ScreenOffset.X + HIDDEN_PANEL_MARGIN);
    Panel->SetRenderTranslation(FVector2D(Offset, 0));
    Panel->SetRenderOpacity(FMath::Clamp(Shown * 1.5f, 0.f, 1.f));
}

bool UControlsHintWidget::IsGamePaused() const
{
    return UGameplayStatics::IsGamePaused(this);
}
