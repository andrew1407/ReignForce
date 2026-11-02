// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Settings/GameSettingsWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Algo/Transform.h"
#include "Framework/Application/SlateApplication.h"

#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

#include "Settings/ReignForceGameUserSettings.h"
#include "UI/Common/MenuLayoutUtils.h"
#include "UI/Settings/KeyBindingsWidget.h"
#include "UI/Settings/GamepadSchemeWidget.h"


using namespace MenuLayoutUtils;

namespace
{
    constexpr int32 OPTION_FONT_SIZE = 22;
    constexpr int32 HEADER_FONT_SIZE = 18;
    constexpr float SCREEN_MARGIN = 12;
    /** Half of the gap on each side of a row: 16 between the rows, as between the main & pause menus items */
    constexpr float OPTION_ROW_GAP = 8;
    constexpr float ARROW_GAP = 12;
    const FLinearColor HEADER_COLOR(1, 1, 1, .75f);
    const FLinearColor DISABLED_COLOR(1, 1, 1, .3f);
    const FLinearColor DISABLED_LABEL_COLOR(1, 1, 1, .35f);

    struct FSettingsSection
    {
        const TCHAR* Title;
        TArray<EGameSettingsOption> Options;
    };

    /** Menu sections with their option rows in order (the controls one ends with the submenus) */
    const FSettingsSection SECTIONS[] = {
        { TEXT("Graphics"), { EGameSettingsOption::WINDOW_MODE, EGameSettingsOption::RESOLUTION, EGameSettingsOption::QUALITY, EGameSettingsOption::VSYNC, EGameSettingsOption::FRAME_RATE } },
        { TEXT("Audio"), { EGameSettingsOption::MASTER_VOLUME, EGameSettingsOption::MUSIC_VOLUME, EGameSettingsOption::EFFECTS_VOLUME } },
        { TEXT("Controls"), { EGameSettingsOption::LOOK_SENSITIVITY, EGameSettingsOption::INVERT_MOUSE_X, EGameSettingsOption::INVERT_MOUSE_Y, EGameSettingsOption::INVERT_GAMEPAD_X, EGameSettingsOption::INVERT_GAMEPAD_Y } },
    };
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

    Options = FGameSettingsOptions::Of(UReignForceGameUserSettings::GetReignForceGameUserSettings());
    RowViews.SetNum(static_cast<int32>(EGameSettingsOption::COUNT));
    ButtonsStyleTemplate = StyleTemplate;
    const FSlateFontInfo RowFont = MakeFont(StyleTemplate, OPTION_FONT_SIZE);
    const FSlateFontInfo HeaderFont = MakeFont(StyleTemplate, HEADER_FONT_SIZE);

    UVerticalBox* Column = ConstructMenuColumn(WidgetTree, FText::FromString(TEXT("Settings")), MakeFont(StyleTemplate, MENU_TITLE_FONT_SIZE), MenuWidth, BackgroundColor);

    // Options scroll on short screens, the actions (apply & back) under them are always visible
    auto List = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Options"));
    if (UVerticalBoxSlot* ListSlot = Column->AddChildToVerticalBox(List))
        ListSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    WrapInScrollBox(WidgetTree, List);
    LimitHeightToScreen(Cast<USizeBox>(Column->GetParent()), SCREEN_MARGIN);

    for (const auto& [Title, SectionOptions] : SECTIONS)
    {
        UTextBlock* Header = MakeText(WidgetTree, FText::FromString(Title), HeaderFont, HEADER_COLOR);
        if (UVerticalBoxSlot* HeaderSlot = List->AddChildToVerticalBox(Header))
            HeaderSlot->SetPadding(FMargin(4, List->GetChildrenCount() > 1 ? 6 : 0, 0, 0));
        for (EGameSettingsOption Option : SectionOptions) AddOptionRow(List, Option, RowFont);
    }

    // Controls are separate menus: key bindings (changed keys are applied right away) & gamepad scheme
    UButton* ControlsButton = AddSubmenuButton(List, TEXT("Keyboard & mouse controls"), TEXT("ControlsButton"), RowFont);
    ControlsButton->OnClicked.AddDynamic(this, &UGameSettingsWidget::HandleControlsClicked);
    GamepadSchemeButton = AddSubmenuButton(List, TEXT("Gamepad controls"), TEXT("GamepadSchemeButton"), RowFont);
    GamepadSchemeButton->OnClicked.AddDynamic(this, &UGameSettingsWidget::HandleGamepadSchemeClicked);

    // Up & down go item by item through the options: the scroll box brings the next one into view (the default navigation
    // only sees the items on the screen, so from the last visible row it jumped straight to the actions under the scroll box).
    // The last one leads to the actions (see RefreshApplyState)
    TArray<UWidget*> ListButtons = List->GetAllChildren().FilterByPredicate([] (const UWidget* Child) { return Child->IsA<UButton>(); });
    constexpr bool bStopAfterLast = false;
    ChainVerticalNavigation(ListButtons, bStopAfterLast);

    AddActions(Column, RowFont);
    RefreshApplyState();
}

void UGameSettingsWidget::AddOptionRow(UVerticalBox* List, EGameSettingsOption Option, const FSlateFontInfo& Font)
{
    const FSlateColor TextColor = FLinearColor::White;
    auto Content = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    FOptionRowView& View = RowViews[static_cast<int32>(Option)];
    View.ValueText = MakeText(WidgetTree, FText::GetEmpty(), Font, TextColor);
    // Both arrows are always shown, the one which can't change the value further is dimmed
    View.PreviousArrow = MakeText(WidgetTree, FText::FromString(TEXT("<")), Font, TextColor);
    View.NextArrow = MakeText(WidgetTree, FText::FromString(TEXT(">")), Font, TextColor);

    if (UHorizontalBoxSlot* LabelSlot = Content->AddChildToHorizontalBox(MakeText(WidgetTree, Options.GetRow(Option).Label, Font, TextColor)))
    {
        LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        LabelSlot->SetVerticalAlignment(VAlign_Center);
    }
    const TPair<UTextBlock*, FMargin> ValueParts[] = {
        { View.PreviousArrow.Get(), FMargin(0, 0, ARROW_GAP, 0) },
        { View.ValueText.Get(), FMargin(0) },
        { View.NextArrow.Get(), FMargin(ARROW_GAP, 0, 0, 0) },
    };
    for (const auto& [Part, PartPadding] : ValueParts)
    {
        if (UHorizontalBoxSlot* PartSlot = Content->AddChildToHorizontalBox(Part))
        {
            PartSlot->SetVerticalAlignment(VAlign_Center);
            PartSlot->SetPadding(PartPadding);
        }
    }

    UButton* Button = MakeButton(WidgetTree, ButtonsStyleTemplate.Get(), Content, *FString::Printf(TEXT("Option_%d"), static_cast<int32>(Option)));
    Button->OnClicked.AddDynamic(this, &UGameSettingsWidget::HandleOptionClicked);
    FCustomWidgetNavigationDelegate OnNavigation;
    OnNavigation.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UGameSettingsWidget, HandleOptionNavigation));
    Button->SetNavigationRuleCustom(EUINavigation::Left, OnNavigation);
    Button->SetNavigationRuleCustom(EUINavigation::Right, OnNavigation);
    if (UVerticalBoxSlot* RowSlot = List->AddChildToVerticalBox(Button))
        RowSlot->SetPadding(FMargin(0, OPTION_ROW_GAP));

    View.Button = Button;
    RegisterFocusableButton(Button);
    RefreshRowText(Option);
}

UButton* UGameSettingsWidget::AddSubmenuButton(UVerticalBox* List, const TCHAR* Label, FName Name, const FSlateFontInfo& Font)
{
    const FSlateColor TextColor = FLinearColor::White;
    auto Content = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    if (UHorizontalBoxSlot* LabelSlot = Content->AddChildToHorizontalBox(MakeText(WidgetTree, FText::FromString(Label), Font, TextColor)))
        LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    Content->AddChildToHorizontalBox(MakeText(WidgetTree, FText::FromString(TEXT("...")), Font, TextColor));

    UButton* Button = MakeButton(WidgetTree, ButtonsStyleTemplate.Get(), Content, Name);
    RegisterFocusableButton(Button);
    if (UVerticalBoxSlot* ButtonSlot = List->AddChildToVerticalBox(Button))
        ButtonSlot->SetPadding(FMargin(0, OPTION_ROW_GAP));
    return Button;
}

void UGameSettingsWidget::AddActions(UVerticalBox* Column, const FSlateFontInfo& Font)
{
    const FSlateColor TextColor = FLinearColor::White;
    auto Actions = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Actions"));
    ApplyLabel = MakeText(WidgetTree, FText::FromString(TEXT("Apply")), Font, TextColor);
    ApplyButton = MakeButton(WidgetTree, ButtonsStyleTemplate.Get(), ApplyLabel, TEXT("ApplyButton"));
    BackButton = MakeButton(WidgetTree, ButtonsStyleTemplate.Get(), MakeText(WidgetTree, FText::FromString(TEXT("Back")), Font, TextColor), TEXT("BackButton"));
    ApplyButton->OnClicked.AddDynamic(this, &UGameSettingsWidget::ApplyChanges);
    BackButton->OnClicked.AddDynamic(this, &UGameSettingsWidget::HandleBackClicked);

    // The pair spans the same width as the rows above: the gap is only between the buttons
    const TPair<UButton*, FMargin> ActionButtons[] = {
        { ApplyButton.Get(), FMargin(0, 0, 6, 0) },
        { BackButton.Get(), FMargin(6, 0, 0, 0) },
    };
    for (const auto& [Button, ButtonPadding] : ActionButtons)
    {
        if (UHorizontalBoxSlot* ActionSlot = Actions->AddChildToHorizontalBox(Button))
        {
            ActionSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            ActionSlot->SetPadding(ButtonPadding);
        }
        RegisterFocusableButton(Button);
        // The actions row is narrower & split in two, so the way to it and back is set explicitly
        // (down from the options & left from Back lead to Apply only while it's enabled, see RefreshApplyState)
        Button->SetNavigationRuleExplicit(EUINavigation::Up, GamepadSchemeButton);
        Button->SetNavigationRuleBase(EUINavigation::Down, EUINavigationRule::Stop);
    }
    SetBackActionButton(BackButton);
    ApplyButton->SetNavigationRuleExplicit(EUINavigation::Right, BackButton);
    ApplyButton->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Stop);
    BackButton->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Stop);

    // Same inset as the scrolled options, so the actions line up with them
    if (UVerticalBoxSlot* ActionsSlot = Column->AddChildToVerticalBox(Actions))
        ActionsSlot->SetPadding(FMargin(SCROLLED_CONTENT_INSET, 8, SCROLLED_CONTENT_INSET, 6));
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
    constexpr bool bWrap = false;
    const int32 Delta = Direction == EUINavigation::Left ? -1 : Direction == EUINavigation::Right ? 1 : 0;
    ChangeOption(FindActiveOption(bPreferHovered), Delta, bWrap);
    // Focus stays on the option row
    return nullptr;
}

void UGameSettingsWidget::HandleOptionClicked()
{
    constexpr bool bPreferHovered = true;
    constexpr bool bWrap = true;
    ChangeOption(FindActiveOption(bPreferHovered), 1, bWrap);
}

void UGameSettingsWidget::ChangeOption(EGameSettingsOption Option, int32 Delta, bool bWrap)
{
    if (!Options.Change(Option, Delta, bWrap)) return;

    RefreshRowText(Option);
    RefreshApplyState();
    UReignForceGameUserSettings* Settings = UReignForceGameUserSettings::GetReignForceGameUserSettings();
    if (Options.GetRow(Option).bLive && IsValid(Settings))
    {
        Options.PreviewTo(Settings, Option);
        Settings->ApplyAudioSettings(this);
    }

    const UButton* Button = RowViews[static_cast<int32>(Option)].Button.Get();
    if (IsValid(Button) && FSlateApplication::IsInitialized())
        FSlateApplication::Get().PlaySound(Button->GetStyle().PressedSlateSound);
}

void UGameSettingsWidget::ApplyChanges()
{
    if (!HasPendingChanges()) return;
    UReignForceGameUserSettings* Settings = UReignForceGameUserSettings::GetReignForceGameUserSettings();
    if (IsValid(Settings))
    {
        Options.WriteTo(Settings);
        constexpr bool bCheckForCommandLineOverrides = false;
        Settings->ApplySettings(bCheckForCommandLineOverrides);
        // Unconfirmed window mode & resolution are reverted to the last confirmed ones on the next launch
        Settings->ConfirmVideoMode();
        Settings->SaveSettings();
        Settings->ApplyAudioSettings(this);
    }
    // Stays in the settings: the applied values become the new baseline, Back no longer reverts them
    Options.MarkApplied();
    RefreshApplyState();
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

void UGameSettingsWidget::HandleControlsClicked()
{
    auto KeyBindings = IsValid(KeyBindingsMenuClass) ? CreateWidget<UKeyBindingsWidget>(GetOwningPlayer(), KeyBindingsMenuClass) : nullptr;
    if (!IsValid(KeyBindings)) return;
    KeyBindings->Setup(ButtonsStyleTemplate.Get());
    PushChildMenu(KeyBindings);
}

void UGameSettingsWidget::HandleGamepadSchemeClicked()
{
    auto GamepadScheme = IsValid(GamepadSchemeMenuClass) ? CreateWidget<UGamepadSchemeWidget>(GetOwningPlayer(), GamepadSchemeMenuClass) : nullptr;
    if (!IsValid(GamepadScheme)) return;
    GamepadScheme->Setup(ButtonsStyleTemplate.Get());
    PushChildMenu(GamepadScheme);
}

void UGameSettingsWidget::HandleBackClicked()
{
    DiscardChanges();
    RequestClose();
}

void UGameSettingsWidget::RefreshRowText(EGameSettingsOption Option)
{
    const FGameSettingsOptionRow& Row = Options.GetRow(Option);
    const FOptionRowView& View = RowViews[static_cast<int32>(Option)];
    if (!(View.ValueText.IsValid() && Row.Names.IsValidIndex(Row.Index))) return;
    View.ValueText->SetText(Row.Names[Row.Index]);
    if (View.PreviousArrow.IsValid()) View.PreviousArrow->SetColorAndOpacity(Row.HasPrevious() ? FLinearColor::White : DISABLED_COLOR);
    if (View.NextArrow.IsValid()) View.NextArrow->SetColorAndOpacity(Row.HasNext() ? FLinearColor::White : DISABLED_COLOR);
}

EGameSettingsOption UGameSettingsWidget::FindActiveOption(bool bPreferHovered) const
{
    TArray<UButton*> Buttons;
    Algo::Transform(RowViews, Buttons, [] (const FOptionRowView& View) { return View.Button.Get(); });
    const int32 Index = FindActiveButton(Buttons, bPreferHovered);
    return Index == INDEX_NONE ? EGameSettingsOption::COUNT : static_cast<EGameSettingsOption>(Index);
}

void UGameSettingsWidget::RefreshApplyState()
{
    if (!IsValid(ApplyButton) || !IsValid(BackButton)) return;
    const bool bChanged = HasPendingChanges();
    // Leaving the focus on a disabled Apply would stop keyboard & gamepad navigation
    const bool bApplyFocused = ApplyButton->HasAnyUserFocus();
    ApplyButton->SetIsEnabled(bChanged);
    if (IsValid(ApplyLabel)) ApplyLabel->SetColorAndOpacity(bChanged ? FLinearColor::White : DISABLED_LABEL_COLOR);
    if (!bChanged && bApplyFocused) FocusButton(BackButton);

    UWidget* BelowOptions = bChanged ? static_cast<UWidget*>(ApplyButton) : static_cast<UWidget*>(BackButton);
    if (IsValid(GamepadSchemeButton)) GamepadSchemeButton->SetNavigationRuleExplicit(EUINavigation::Down, BelowOptions);
    if (bChanged) BackButton->SetNavigationRuleExplicit(EUINavigation::Left, ApplyButton);
    else BackButton->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Stop);
}
