// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Settings/GameSettingsWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Framework/Application/SlateApplication.h"

#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

#include "Settings/ReignForceGameUserSettings.h"
#include "UI/Settings/KeyBindingsWidget.h"
#include "UI/Settings/GamepadSchemeWidget.h"


namespace
{
    constexpr int32 VOLUME_STEPS = 10;
    constexpr int32 SENSITIVITY_PERCENT_STEP = 10;

    FText Percent(int32 Value) { return FText::FromString(FString::Printf(TEXT("%d%%"), Value)); }
}

UGameSettingsWidget::UGameSettingsWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    MenuWidth = 760;
    BackgroundColor = FLinearColor(0, 0, 0, .85f);
    bBuilt = false;
    KeyBindingsMenuClass = UKeyBindingsWidget::StaticClass();
    GamepadSchemeMenuClass = UGamepadSchemeWidget::StaticClass();
}

TSharedRef<SWidget> UGameSettingsWidget::RebuildWidget()
{
    // Created without a style template, the engine default button style is used
    if (!bBuilt) Setup(nullptr);
    return Super::RebuildWidget();
}

void UGameSettingsWidget::Setup(UButton* StyleTemplate)
{
    if (bBuilt || !IsValid(WidgetTree)) return;
    bBuilt = true;

    UReignForceGameUserSettings* Settings = UReignForceGameUserSettings::GetReignForceGameUserSettings();
    ReadOptions(Settings);

    const FSlateFontInfo TitleFont = MakeFont(StyleTemplate, 44);
    const FSlateFontInfo RowFont = MakeFont(StyleTemplate, 22);
    const FSlateColor TextColor = FLinearColor::White;
    ButtonsStyleTemplate = StyleTemplate;

    UVerticalBox* Column = ConstructMenuColumn(FText::FromString(TEXT("Settings")), TitleFont, MenuWidth, BackgroundColor);

    // Options scroll on short screens, the actions (apply & back) under them are always visible
    auto Options = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Options"));
    if (UVerticalBoxSlot* OptionsSlot = Column->AddChildToVerticalBox(Options))
        OptionsSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    WrapInScrollBox(Options);
    constexpr float ScreenMargin = 12;
    LimitHeightToScreen(Cast<USizeBox>(Column->GetParent()), ScreenMargin);

    const FSlateFontInfo HeaderFont = MakeFont(StyleTemplate, 18);
    // Half of the gap on each side of a row: 16 between the rows, as between the main & pause menus items
    constexpr float RowGap = 8;
    bool bFirstSection = true;
    auto AddSectionHeader = [&] (const TCHAR* Title)
    {
        UTextBlock* Header = MakeText(FText::FromString(Title), HeaderFont, FLinearColor(1, 1, 1, .75f));
        if (UVerticalBoxSlot* HeaderSlot = Options->AddChildToVerticalBox(Header))
            HeaderSlot->SetPadding(FMargin(4, bFirstSection ? 0 : 6, 0, 0));
        bFirstSection = false;
    };

    auto AddOptionRow = [&] (int32 RowIndex)
    {
        FOptionRow& Row = Rows[RowIndex];
        auto Content = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
        UTextBlock* Label = MakeText(Row.Label, RowFont, TextColor);
        UTextBlock* Value = MakeText(FText::GetEmpty(), RowFont, TextColor);
        // Both arrows are always shown, the one which can't change the value further is dimmed
        UTextBlock* PreviousArrow = MakeText(FText::FromString(TEXT("<")), RowFont, TextColor);
        UTextBlock* NextArrow = MakeText(FText::FromString(TEXT(">")), RowFont, TextColor);
        if (UHorizontalBoxSlot* LabelSlot = Content->AddChildToHorizontalBox(Label))
        {
            LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            LabelSlot->SetVerticalAlignment(VAlign_Center);
        }
        const TPair<UTextBlock*, FMargin> ValueParts[] = {
            { PreviousArrow, FMargin(0, 0, 12, 0) },
            { Value, FMargin(0) },
            { NextArrow, FMargin(12, 0, 0, 0) },
        };
        for (const auto& [Part, PartPadding] : ValueParts)
        {
            if (UHorizontalBoxSlot* PartSlot = Content->AddChildToHorizontalBox(Part))
            {
                PartSlot->SetVerticalAlignment(VAlign_Center);
                PartSlot->SetPadding(PartPadding);
            }
        }

        UButton* Button = MakeButton(StyleTemplate, Content, *FString::Printf(TEXT("Option_%d"), RowIndex));
        Button->OnClicked.AddDynamic(this, &UGameSettingsWidget::HandleOptionClicked);

        FCustomWidgetNavigationDelegate OnNavigation;
        OnNavigation.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UGameSettingsWidget, HandleOptionNavigation));
        Button->SetNavigationRuleCustom(EUINavigation::Left, OnNavigation);
        Button->SetNavigationRuleCustom(EUINavigation::Right, OnNavigation);

        if (UVerticalBoxSlot* RowSlot = Options->AddChildToVerticalBox(Button))
            RowSlot->SetPadding(FMargin(0, RowGap));

        Row.Button = Button;
        Row.ValueText = Value;
        Row.PreviousArrow = PreviousArrow;
        Row.NextArrow = NextArrow;
        RegisterFocusableButton(Button);
        RefreshRowText(RowIndex);
    };

    // Controls are separate menus: key bindings (changed keys are applied right away) & gamepad scheme
    auto MakeSubmenuButton = [&] (const TCHAR* Label, FName Name)
    {
        auto Content = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
        if (UHorizontalBoxSlot* LabelSlot = Content->AddChildToHorizontalBox(MakeText(FText::FromString(Label), RowFont, TextColor)))
            LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        Content->AddChildToHorizontalBox(MakeText(FText::FromString(TEXT("...")), RowFont, TextColor));
        UButton* Button = MakeButton(StyleTemplate, Content, Name);
        RegisterFocusableButton(Button);
        if (UVerticalBoxSlot* ButtonSlot = Options->AddChildToVerticalBox(Button))
            ButtonSlot->SetPadding(FMargin(0, RowGap));
        return Button;
    };

    AddSectionHeader(TEXT("Graphics"));
    for (int32 RowIndex : { WindowMode, Resolution, Quality, VSync, FrameRate }) AddOptionRow(RowIndex);

    AddSectionHeader(TEXT("Audio"));
    for (int32 RowIndex : { MasterVolume, MusicVolume, EffectsVolume }) AddOptionRow(RowIndex);

    AddSectionHeader(TEXT("Controls"));
    AddOptionRow(LookSensitivity);
    ControlsButton = MakeSubmenuButton(TEXT("Keyboard & mouse controls"), TEXT("ControlsButton"));
    ControlsButton->OnClicked.AddDynamic(this, &UGameSettingsWidget::HandleControlsClicked);
    GamepadSchemeButton = MakeSubmenuButton(TEXT("Gamepad controls"), TEXT("GamepadSchemeButton"));
    GamepadSchemeButton->OnClicked.AddDynamic(this, &UGameSettingsWidget::HandleGamepadSchemeClicked);

    auto Actions = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Actions"));
    ApplyLabel = MakeText(FText::FromString(TEXT("Apply")), RowFont, TextColor);
    ApplyButton = MakeButton(StyleTemplate, ApplyLabel, TEXT("ApplyButton"));
    BackButton = MakeButton(StyleTemplate, MakeText(FText::FromString(TEXT("Back")), RowFont, TextColor), TEXT("BackButton"));
    ApplyButton->OnClicked.AddDynamic(this, &UGameSettingsWidget::HandleApplyClicked);
    BackButton->OnClicked.AddDynamic(this, &UGameSettingsWidget::HandleBackClicked);
    // The pair spans the same width as the rows above: the gap is only between the buttons
    if (UHorizontalBoxSlot* ApplySlot = Actions->AddChildToHorizontalBox(ApplyButton))
    {
        ApplySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        ApplySlot->SetPadding(FMargin(0, 0, 6, 0));
    }
    if (UHorizontalBoxSlot* BackSlot = Actions->AddChildToHorizontalBox(BackButton))
    {
        BackSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        BackSlot->SetPadding(FMargin(6, 0, 0, 0));
    }
    RegisterFocusableButton(ApplyButton);
    RegisterFocusableButton(BackButton);
    // Same inset as the scrolled options, so the actions line up with them
    if (UVerticalBoxSlot* ActionsSlot = Column->AddChildToVerticalBox(Actions))
        ActionsSlot->SetPadding(FMargin(SCROLLED_CONTENT_INSET, 8, SCROLLED_CONTENT_INSET, 6));

    // The actions row is narrower & split in two, so the way to it and back is set explicitly
    // (down from the options & left from Back lead to Apply only while it's enabled, see RefreshApplyState)
    ApplyButton->SetNavigationRuleExplicit(EUINavigation::Up, GamepadSchemeButton);
    BackButton->SetNavigationRuleExplicit(EUINavigation::Up, GamepadSchemeButton);
    ApplyButton->SetNavigationRuleExplicit(EUINavigation::Right, BackButton);
    ApplyButton->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Stop);
    BackButton->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Stop);
    ApplyButton->SetNavigationRuleBase(EUINavigation::Down, EUINavigationRule::Stop);
    BackButton->SetNavigationRuleBase(EUINavigation::Down, EUINavigationRule::Stop);

    RefreshApplyState();
}

bool UGameSettingsWidget::HandleBackAction()
{
    HandleBackClicked();
    return true;
}

UWidget* UGameSettingsWidget::HandleOptionNavigation(EUINavigation Direction)
{
    // Arrows & gamepad change the focused row, never the one under a resting mouse cursor
    constexpr bool bPreferHovered = false;
    const int32 RowIndex = FindActiveRow(bPreferHovered);
    constexpr bool bWrap = false;
    if (Direction == EUINavigation::Left) ChangeOption(RowIndex, -1, bWrap);
    else if (Direction == EUINavigation::Right) ChangeOption(RowIndex, 1, bWrap);
    // Focus stays on the option row
    return nullptr;
}

void UGameSettingsWidget::HandleOptionClicked()
{
    constexpr bool bWrap = true;
    constexpr bool bPreferHovered = true;
    ChangeOption(FindActiveRow(bPreferHovered), 1, bWrap);
}

void UGameSettingsWidget::HandleApplyClicked()
{
    if (!HasPendingChanges()) return;
    UReignForceGameUserSettings* Settings = UReignForceGameUserSettings::GetReignForceGameUserSettings();
    if (IsValid(Settings) && Rows.Num() == EOption::Count)
    {
        Settings->SetFullscreenMode(static_cast<EWindowMode::Type>(Rows[WindowMode].GetValue()));
        const int32 ResolutionIndex = Rows[Resolution].GetValue();
        if (Resolutions.IsValidIndex(ResolutionIndex)) Settings->SetScreenResolution(Resolutions[ResolutionIndex]);
        const int32 QualityLevel = Rows[Quality].GetValue();
        if (QualityLevel >= 0) Settings->SetOverallScalabilityLevel(QualityLevel);
        Settings->SetVSyncEnabled(Rows[VSync].GetValue() != 0);
        Settings->SetFrameRateLimit(Rows[FrameRate].GetValue());
        Settings->SetLookSensitivity(Rows[LookSensitivity].GetValue() / 100.f);
        Settings->SetMasterVolume(Rows[MasterVolume].GetValue() / static_cast<float>(VOLUME_STEPS));
        Settings->SetMusicVolume(Rows[MusicVolume].GetValue() / static_cast<float>(VOLUME_STEPS));
        Settings->SetEffectsVolume(Rows[EffectsVolume].GetValue() / static_cast<float>(VOLUME_STEPS));

        constexpr bool bCheckForCommandLineOverrides = false;
        Settings->ApplySettings(bCheckForCommandLineOverrides);
        // Unconfirmed window mode & resolution are reverted to the last confirmed ones on the next launch
        Settings->ConfirmVideoMode();
        Settings->SaveSettings();
        Settings->ApplyAudioSettings(this);
    }
    // Stays in the settings: the applied values become the new baseline, Back no longer reverts them
    for (FOptionRow& Row : Rows) Row.AppliedIndex = Row.Index;
    RefreshApplyState();
}

void UGameSettingsWidget::HandleControlsClicked()
{
    if (!IsValid(KeyBindingsMenuClass)) return;
    auto KeyBindings = CreateWidget<UKeyBindingsWidget>(GetOwningPlayer(), KeyBindingsMenuClass);
    if (!IsValid(KeyBindings)) return;
    KeyBindings->Setup(ButtonsStyleTemplate.Get());
    PushChildMenu(KeyBindings);
}

void UGameSettingsWidget::HandleGamepadSchemeClicked()
{
    if (!IsValid(GamepadSchemeMenuClass)) return;
    auto GamepadScheme = CreateWidget<UGamepadSchemeWidget>(GetOwningPlayer(), GamepadSchemeMenuClass);
    if (!IsValid(GamepadScheme)) return;
    GamepadScheme->Setup(ButtonsStyleTemplate.Get());
    PushChildMenu(GamepadScheme);
}

void UGameSettingsWidget::HandleBackClicked()
{
    DiscardChanges();
    RequestClose();
}

void UGameSettingsWidget::ReadOptions(const UReignForceGameUserSettings* Settings)
{
    Rows.SetNum(EOption::Count);
    auto Select = [] (FOptionRow& Row, int32 Value)
    {
        int32 Closest = 0;
        for (int32 i = 0; i < Row.Values.Num(); ++i)
        {
            if (FMath::Abs(Row.Values[i] - Value) < FMath::Abs(Row.Values[Closest] - Value)) Closest = i;
        }
        Row.Index = Closest;
    };

    FOptionRow& WindowModeRow = Rows[WindowMode];
    WindowModeRow.Label = FText::FromString(TEXT("Window mode"));
    WindowModeRow.Names = { FText::FromString(TEXT("Fullscreen")), FText::FromString(TEXT("Borderless")), FText::FromString(TEXT("Windowed")) };
    WindowModeRow.Values = { EWindowMode::Fullscreen, EWindowMode::WindowedFullscreen, EWindowMode::Windowed };

    UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
    const FIntPoint CurrentResolution = IsValid(Settings) ? Settings->GetScreenResolution() : FIntPoint(1920, 1080);
    Resolutions.AddUnique(CurrentResolution);
    Resolutions.Sort([] (const FIntPoint& A, const FIntPoint& B) { return A.X == B.X ? A.Y < B.Y : A.X < B.X; });
    FOptionRow& ResolutionRow = Rows[Resolution];
    ResolutionRow.Label = FText::FromString(TEXT("Resolution"));
    for (int32 i = 0; i < Resolutions.Num(); ++i)
    {
        ResolutionRow.Names.Add(FText::FromString(FString::Printf(TEXT("%d x %d"), Resolutions[i].X, Resolutions[i].Y)));
        ResolutionRow.Values.Add(i);
    }

    FOptionRow& QualityRow = Rows[Quality];
    QualityRow.Label = FText::FromString(TEXT("Graphics quality"));
    QualityRow.Names = { FText::FromString(TEXT("Low")), FText::FromString(TEXT("Medium")), FText::FromString(TEXT("High")), FText::FromString(TEXT("Epic")), FText::FromString(TEXT("Cinematic")) };
    QualityRow.Values = { 0, 1, 2, 3, 4 };
    const int32 QualityLevel = IsValid(Settings) ? Settings->GetOverallScalabilityLevel() : 3;
    if (QualityLevel < 0)
    {
        // Per-group custom scalability is kept untouched until another level is picked
        QualityRow.Names.Insert(FText::FromString(TEXT("Custom")), 0);
        QualityRow.Values.Insert(-1, 0);
    }

    FOptionRow& VSyncRow = Rows[VSync];
    VSyncRow.Label = FText::FromString(TEXT("VSync"));
    VSyncRow.Names = { FText::FromString(TEXT("Off")), FText::FromString(TEXT("On")) };
    VSyncRow.Values = { 0, 1 };

    FOptionRow& FrameRateRow = Rows[FrameRate];
    FrameRateRow.Label = FText::FromString(TEXT("Frame rate limit"));
    FrameRateRow.Values = { 30, 60, 120, 144, 240, 0 };
    for (int32 Limit : FrameRateRow.Values)
        FrameRateRow.Names.Add(Limit > 0 ? FText::AsNumber(Limit) : FText::FromString(TEXT("Unlimited")));

    FOptionRow& SensitivityRow = Rows[LookSensitivity];
    SensitivityRow.Label = FText::FromString(TEXT("Camera sensitivity"));
    for (int32 Value = 20; Value <= 300; Value += SENSITIVITY_PERCENT_STEP)
    {
        SensitivityRow.Values.Add(Value);
        SensitivityRow.Names.Add(Percent(Value));
    }

    const TPair<EOption, FText> Volumes[] = {
        { MasterVolume, FText::FromString(TEXT("Master volume")) },
        { MusicVolume, FText::FromString(TEXT("Music volume")) },
        { EffectsVolume, FText::FromString(TEXT("Game sounds volume")) },
    };
    for (const auto& [Option, Label] : Volumes)
    {
        FOptionRow& Row = Rows[Option];
        Row.Label = Label;
        Row.bLive = true;
        for (int32 Step = 0; Step <= VOLUME_STEPS; ++Step)
        {
            Row.Values.Add(Step);
            Row.Names.Add(Percent(Step * 100 / VOLUME_STEPS));
        }
    }

    if (!IsValid(Settings)) return;
    Select(WindowModeRow, Settings->GetFullscreenMode());
    Select(ResolutionRow, Resolutions.IndexOfByKey(CurrentResolution));
    Select(QualityRow, QualityLevel);
    Select(VSyncRow, Settings->IsVSyncEnabled() ? 1 : 0);
    const int32 FrameRateLimit = FMath::RoundToInt(Settings->GetFrameRateLimit());
    if (FrameRateLimit > 0) Select(FrameRateRow, FrameRateLimit);
    else FrameRateRow.Index = FrameRateRow.Values.Num() - 1;
    Select(SensitivityRow, FMath::RoundToInt(Settings->GetLookSensitivity() * 100));
    Select(Rows[MasterVolume], FMath::RoundToInt(Settings->GetMasterVolume() * VOLUME_STEPS));
    Select(Rows[MusicVolume], FMath::RoundToInt(Settings->GetMusicVolume() * VOLUME_STEPS));
    Select(Rows[EffectsVolume], FMath::RoundToInt(Settings->GetEffectsVolume() * VOLUME_STEPS));
    for (FOptionRow& Row : Rows) Row.AppliedIndex = Row.Index;
}

void UGameSettingsWidget::ChangeOption(int32 RowIndex, int32 Delta, bool bWrap)
{
    if (!Rows.IsValidIndex(RowIndex)) return;
    FOptionRow& Row = Rows[RowIndex];
    const int32 Count = Row.Values.Num();
    if (Count == 0) return;

    const int32 Next = Row.Index + Delta;
    const int32 Index = bWrap ? (Next % Count + Count) % Count : FMath::Clamp(Next, 0, Count - 1);
    if (Index == Row.Index) return;
    Row.Index = Index;

    RefreshRowText(RowIndex);
    RefreshApplyState();
    if (Row.bLive) PreviewOption(RowIndex);
    if (Row.Button.IsValid() && FSlateApplication::IsInitialized())
        FSlateApplication::Get().PlaySound(Row.Button->GetStyle().PressedSlateSound);
}

void UGameSettingsWidget::PreviewOption(int32 RowIndex)
{
    UReignForceGameUserSettings* Settings = UReignForceGameUserSettings::GetReignForceGameUserSettings();
    if (!IsValid(Settings) || !Rows.IsValidIndex(RowIndex)) return;

    const float Volume = Rows[RowIndex].GetValue() / static_cast<float>(VOLUME_STEPS);
    if (RowIndex == MasterVolume) Settings->SetMasterVolume(Volume);
    else if (RowIndex == MusicVolume) Settings->SetMusicVolume(Volume);
    else if (RowIndex == EffectsVolume) Settings->SetEffectsVolume(Volume);
    Settings->ApplyAudioSettings(this);
}

void UGameSettingsWidget::RefreshRowText(int32 RowIndex)
{
    if (!Rows.IsValidIndex(RowIndex)) return;
    const FOptionRow& Row = Rows[RowIndex];
    if (!(Row.ValueText.IsValid() && Row.Names.IsValidIndex(Row.Index))) return;
    Row.ValueText->SetText(Row.Names[Row.Index]);

    const FLinearColor Enabled = FLinearColor::White;
    const FLinearColor Disabled(1, 1, 1, .3f);
    if (Row.PreviousArrow.IsValid()) Row.PreviousArrow->SetColorAndOpacity(Row.Index > 0 ? Enabled : Disabled);
    if (Row.NextArrow.IsValid()) Row.NextArrow->SetColorAndOpacity(Row.Index < Row.Names.Num() - 1 ? Enabled : Disabled);
}

int32 UGameSettingsWidget::FindActiveRow(bool bPreferHovered) const
{
    // Mouse clicks target the hovered row, keyboard & gamepad - the focused one
    for (int32 i = 0; bPreferHovered && i < Rows.Num(); ++i)
    {
        if (Rows[i].Button.IsValid() && Rows[i].Button->IsHovered()) return i;
    }
    for (int32 i = 0; i < Rows.Num(); ++i)
    {
        if (Rows[i].Button.IsValid() && Rows[i].Button->HasAnyUserFocus()) return i;
    }
    return INDEX_NONE;
}

void UGameSettingsWidget::DiscardChanges()
{
    UReignForceGameUserSettings* Settings = UReignForceGameUserSettings::GetReignForceGameUserSettings();
    if (!IsValid(Settings)) return;
    // Live previewed volumes are restored from the saved config
    constexpr bool bForceReload = true;
    Settings->LoadSettings(bForceReload);
    Settings->ApplyAudioSettings(this);
}

bool UGameSettingsWidget::HasPendingChanges() const
{
    return Rows.ContainsByPredicate([] (const FOptionRow& Row) { return Row.Index != Row.AppliedIndex; });
}

void UGameSettingsWidget::RefreshApplyState()
{
    if (!IsValid(ApplyButton) || !IsValid(BackButton)) return;
    const bool bChanged = HasPendingChanges();
    // Leaving the focus on a disabled Apply would stop keyboard & gamepad navigation
    const bool bApplyFocused = ApplyButton->HasAnyUserFocus();
    ApplyButton->SetIsEnabled(bChanged);
    if (IsValid(ApplyLabel)) ApplyLabel->SetColorAndOpacity(FLinearColor(1, 1, 1, bChanged ? 1.f : .35f));
    if (!bChanged && bApplyFocused) FocusButton(BackButton);

    UWidget* BelowOptions = bChanged ? static_cast<UWidget*>(ApplyButton) : static_cast<UWidget*>(BackButton);
    if (IsValid(GamepadSchemeButton)) GamepadSchemeButton->SetNavigationRuleExplicit(EUINavigation::Down, BelowOptions);
    if (bChanged) BackButton->SetNavigationRuleExplicit(EUINavigation::Left, ApplyButton);
    else BackButton->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Stop);
}
