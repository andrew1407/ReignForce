// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Settings/KeyBindingsWidget.h"
#include "Algo/Find.h"
#include "Algo/Transform.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "GameplayTagContainer.h"

#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "PlayerMappableKeySettings.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#include "Settings/ReignForceSettings.h"
#include "UI/Common/MenuLayoutUtils.h"
#include "UI/Settings/KeyCaptureInputProcessor.h"


using namespace MenuLayoutUtils;

namespace
{
    constexpr int32 BINDING_FONT_SIZE = 20;
    constexpr int32 CATEGORY_FONT_SIZE = 18;
    constexpr int32 RESET_FONT_SIZE = 16;
    constexpr int32 FIRST_INPUT_PROCESSOR = 0;
    const FLinearColor DISABLED_RESET_COLOR(1, 1, 1, .35f);


    bool IsKeyboardOrMouseKey(const FKey& Key)
    {
        return Key.IsValid() && !(Key.IsGamepadKey() || Key.IsTouch() || Key.IsGesture() || Key.IsAxis1D() || Key.IsAxis2D() || Key.IsAxis3D());
    }

    void MapKey(UEnhancedInputUserSettings* Settings, const FPlayerKeyMapping& Mapping, const FKey& Key)
    {
        FMapPlayerKeyArgs Args;
        Args.MappingName = Mapping.GetMappingName();
        Args.Slot = Mapping.GetSlot();
        Args.HardwareDeviceId = Mapping.GetHardwareDeviceId().HardwareDeviceIdentifier;
        Args.NewKey = Key;
        FGameplayTagContainer FailureReason;
        Settings->MapPlayerKey(Args, FailureReason);
    }
}

UKeyBindingsWidget::UKeyBindingsWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    MenuWidth = 760;
    ListHeight = 560;
    BackgroundColor = FLinearColor(0, 0, 0, .85f);
    CategoryColor = FLinearColor(1.f, .8f, .45f);
    ConflictColor = FLinearColor(1.f, .35f, .25f);
    CaptureColor = FLinearColor(1.f, .85f, .2f);
    // Rows are packed tightly, so the focus frame hugs the button
    FocusOutlineStyle.Thickness = 3;
    FocusOutlineStyle.Gap = 0;

    CapturingRow = INDEX_NONE;
    bBuilt = false;
}

TSharedRef<SWidget> UKeyBindingsWidget::RebuildWidget()
{
    // Created without a style template, the engine default button style is used
    if (!bBuilt) Setup(nullptr);
    return Super::RebuildWidget();
}

void UKeyBindingsWidget::NativeDestruct()
{
    StopCapture();
    Super::NativeDestruct();
}

bool UKeyBindingsWidget::HandleBackAction()
{
    if (CapturingRow == INDEX_NONE) return Super::HandleBackAction();
    StopCapture();
    return true;
}

void UKeyBindingsWidget::Setup(UButton* StyleTemplate)
{
    if (bBuilt || !IsValid(WidgetTree)) return;
    bBuilt = true;

    const FSlateFontInfo RowFont = MakeFont(StyleTemplate, BINDING_FONT_SIZE);
    const FSlateFontInfo CategoryFont = MakeFont(StyleTemplate, CATEGORY_FONT_SIZE);
    const FSlateFontInfo ResetFont = MakeFont(StyleTemplate, RESET_FONT_SIZE);
    UVerticalBox* Column = ConstructMenuColumn(WidgetTree, FText::FromString(TEXT("Controls")), MakeFont(StyleTemplate, MENU_TITLE_FONT_SIZE), MenuWidth, BackgroundColor);

    auto ListFrame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ListFrame"));
    ListFrame->SetHeightOverride(ListHeight);
    auto List = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("List"));
    // Keyboard & gamepad navigation keeps the focused row visible
    List->SetScrollWhenFocusChanges(EScrollWhenFocusChanges::AnimatedScroll);
    ListFrame->SetContent(List);
    Column->AddChildToVerticalBox(ListFrame);

    const auto Groups = ReadBindings(GetInputSettings());
    if (Groups.IsEmpty())
        List->AddChild(MakeText(WidgetTree, FText::FromString(TEXT("Key bindings are unavailable")), RowFont, FLinearColor::White));

    for (const auto& [Category, GroupRows] : Groups)
    {
        if (!Category.IsEmpty())
        {
            auto CategorySlot = Cast<UScrollBoxSlot>(List->AddChild(MakeText(WidgetTree, Category, CategoryFont, CategoryColor)));
            if (IsValid(CategorySlot)) CategorySlot->SetPadding(FMargin(4, Rows.IsEmpty() ? 0 : 16, 0, 4));
        }
        for (const FBindingRow& GroupRow : GroupRows) AddBindingRow(List, GroupRow, StyleTemplate, RowFont, ResetFont);
    }

    auto Actions = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Actions"));
    ResetAllButton = MakeButton(WidgetTree, StyleTemplate, MakeText(WidgetTree, FText::FromString(TEXT("Reset all")), RowFont, FLinearColor::White), TEXT("ResetAllButton"));
    BackButton = MakeButton(WidgetTree, StyleTemplate, MakeText(WidgetTree, FText::FromString(TEXT("Back")), RowFont, FLinearColor::White), TEXT("BackButton"));
    ResetAllButton->OnClicked.AddDynamic(this, &UKeyBindingsWidget::HandleResetAllClicked);
    BackButton->OnClicked.AddDynamic(this, &UReignForceActivatableWidget::RequestClose);
    for (UButton* Button : { ResetAllButton.Get(), BackButton.Get() })
    {
        if (UHorizontalBoxSlot* ActionSlot = Actions->AddChildToHorizontalBox(Button))
        {
            ActionSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            ActionSlot->SetPadding(FMargin(6, 0));
        }
        RegisterFocusableButton(Button);
    }
    SetBackActionButton(BackButton);
    if (UVerticalBoxSlot* ActionsSlot = Column->AddChildToVerticalBox(Actions))
        ActionsSlot->SetPadding(FMargin(0, 24, 0, 12));

    RefreshRows();
}

void UKeyBindingsWidget::AddBindingRow(UScrollBox* List, const FBindingRow& GroupRow, UButton* StyleTemplate, const FSlateFontInfo& Font, const FSlateFontInfo& ResetFont)
{
    const int32 RowIndex = Rows.Add(GroupRow);
    FBindingRow& Row = Rows[RowIndex];
    const FSlateColor TextColor = FLinearColor::White;

    auto Content = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    Row.KeyText = MakeText(WidgetTree, FText::GetEmpty(), Font, TextColor);
    if (UHorizontalBoxSlot* LabelSlot = Content->AddChildToHorizontalBox(MakeText(WidgetTree, Row.Label, Font, TextColor)))
    {
        LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        LabelSlot->SetVerticalAlignment(VAlign_Center);
    }
    if (UHorizontalBoxSlot* KeySlot = Content->AddChildToHorizontalBox(Row.KeyText.Get()))
        KeySlot->SetVerticalAlignment(VAlign_Center);

    UButton* Button = MakeButton(WidgetTree, StyleTemplate, Content, *FString::Printf(TEXT("Binding_%d"), RowIndex));
    Row.ResetText = MakeText(WidgetTree, FText::FromString(TEXT("Reset")), ResetFont, TextColor);
    UButton* ResetButton = MakeButton(WidgetTree, StyleTemplate, Row.ResetText.Get(), *FString::Printf(TEXT("Reset_%d"), RowIndex));
    Button->OnClicked.AddDynamic(this, &UKeyBindingsWidget::HandleBindingClicked);
    ResetButton->OnClicked.AddDynamic(this, &UKeyBindingsWidget::HandleResetClicked);

    auto RowBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    if (UHorizontalBoxSlot* ButtonSlot = RowBox->AddChildToHorizontalBox(Button))
        ButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    if (UHorizontalBoxSlot* ResetSlot = RowBox->AddChildToHorizontalBox(ResetButton))
        ResetSlot->SetPadding(FMargin(8, 0, 0, 0));
    auto RowSlot = Cast<UScrollBoxSlot>(List->AddChild(RowBox));
    if (IsValid(RowSlot)) RowSlot->SetPadding(FMargin(4, 3, 12, 3));

    Row.Button = Button;
    Row.ResetButton = ResetButton;
    RegisterFocusableButton(Button);
    RegisterFocusableButton(ResetButton);
}

void UKeyBindingsWidget::HandleBindingClicked()
{
    constexpr bool bResetButton = false;
    const int32 RowIndex = FindActiveRow(bResetButton);
    if (RowIndex != INDEX_NONE) StartCapture(RowIndex);
}

void UKeyBindingsWidget::HandleResetClicked()
{
    constexpr bool bResetButton = true;
    const int32 RowIndex = FindActiveRow(bResetButton);
    UEnhancedInputUserSettings* Settings = GetInputSettings();
    if (!(Rows.IsValidIndex(RowIndex) && IsValid(Settings))) return;

    StopCapture();
    FMapPlayerKeyArgs Args;
    Args.MappingName = Rows[RowIndex].MappingName;
    FGameplayTagContainer FailureReason;
    Settings->ResetAllPlayerKeysInRow(Args, FailureReason);

    // Reset button is disabled for a default key, the focus goes back to the row
    FocusButton(Rows[RowIndex].Button.Get());
    SaveBindings(Settings);
}

void UKeyBindingsWidget::HandleResetAllClicked()
{
    UEnhancedInputUserSettings* Settings = GetInputSettings();
    if (!IsValid(Settings)) return;

    StopCapture();
    FGameplayTagContainer FailureReason;
    Settings->ResetKeyProfileIdToDefault(Settings->GetActiveKeyProfileId(), FailureReason);
    SaveBindings(Settings);
}

UEnhancedInputUserSettings* UKeyBindingsWidget::GetInputSettings() const
{
    auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetOwningLocalPlayer());
    return IsValid(Subsystem) ? Subsystem->GetUserSettings() : nullptr;
}

const FPlayerKeyMapping* UKeyBindingsWidget::FindKeyboardMapping(const UEnhancedInputUserSettings* Settings, FName MappingName) const
{
    const UEnhancedPlayerMappableKeyProfile* Profile = IsValid(Settings) ? Settings->GetActiveKeyProfile() : nullptr;
    const FKeyMappingRow* MappingRow = IsValid(Profile) ? Profile->FindKeyMappingRow(MappingName) : nullptr;
    if (!MappingRow) return nullptr;

    // Each remappable row holds a single keyboard / mouse key in the first slot
    const FPlayerKeyMapping* Found = nullptr;
    for (const FPlayerKeyMapping& Mapping : MappingRow->Mappings)
    {
        if (!IsKeyboardOrMouseKey(Mapping.GetDefaultKey())) continue;
        if (!Found || Mapping.GetSlot() < Found->GetSlot()) Found = &Mapping;
    }
    return Found;
}

TArray<TPair<FText, TArray<UKeyBindingsWidget::FBindingRow>>> UKeyBindingsWidget::ReadBindings(UEnhancedInputUserSettings* Settings) const
{
    TArray<TPair<FText, TArray<FBindingRow>>> Groups;
    const UInputMappingContext* Context = UReignForceSettings::Get()->RemappableInputContext.LoadSynchronous();
    if (!(IsValid(Settings) && IsValid(Context))) return Groups;

    // The menu can be opened before the gameplay (main menu), the player keys rows are created on the registration
    Settings->RegisterInputMappingContext(Context);

    TSet<FName> AddedMappings;
    for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
    {
        if (!(Mapping.IsPlayerMappable() && IsKeyboardOrMouseKey(Mapping.Key))) continue;
        const FName MappingName = Mapping.GetMappingName();
        bool bAlreadyAdded = false;
        AddedMappings.Add(MappingName, &bAlreadyAdded);
        if (MappingName.IsNone() || bAlreadyAdded) continue;

        const UPlayerMappableKeySettings* KeySettings = Mapping.GetPlayerMappableKeySettings();
        const FText Category = IsValid(KeySettings) ? KeySettings->DisplayCategory : FText::GetEmpty();
        auto Group = Groups.FindByPredicate([&] (const TPair<FText, TArray<FBindingRow>>& Pair) { return Pair.Key.EqualTo(Category); });
        if (!Group) Group = &Groups.Emplace_GetRef(Category, TArray<FBindingRow>());

        FBindingRow& Row = Group->Value.AddDefaulted_GetRef();
        Row.MappingName = MappingName;
        Row.Label = IsValid(KeySettings) && !KeySettings->DisplayName.IsEmpty() ? KeySettings->DisplayName : FText::FromName(MappingName);
    }
    return Groups;
}

void UKeyBindingsWidget::StartCapture(int32 RowIndex)
{
    if (!(Rows.IsValidIndex(RowIndex) && FSlateApplication::IsInitialized())) return;
    StopCapture();

    CapturingRow = RowIndex;
    KeyCapture = MakeShared<FKeyCaptureInputProcessor>(FOnKeyCaptured::CreateUObject(this, &UKeyBindingsWidget::HandleCapturedKey));
    FSlateApplication::Get().RegisterInputPreProcessor(KeyCapture, FIRST_INPUT_PROCESSOR);
    RefreshRows();
}

void UKeyBindingsWidget::StopCapture()
{
    if (KeyCapture.IsValid())
    {
        KeyCapture->Detach();
        if (FSlateApplication::IsInitialized()) FSlateApplication::Get().UnregisterInputPreProcessor(KeyCapture);
        KeyCapture.Reset();
    }
    if (CapturingRow == INDEX_NONE) return;
    CapturingRow = INDEX_NONE;
    RefreshRows();
}

void UKeyBindingsWidget::HandleCapturedKey(const FKey& Key)
{
    const int32 RowIndex = CapturingRow;
    if (RowIndex == INDEX_NONE) return;

    // Escape & the gamepad back buttons cancel the capture
    static const FKey CancelKeys[] = { EKeys::Escape, EKeys::Gamepad_FaceButton_Right, EKeys::Gamepad_Special_Right };
    const bool bCancel = Algo::Find(CancelKeys, Key) != nullptr;
    if (bCancel) return StopCapture();
    // Only keyboard & mouse bindings are listed, other keys are skipped while waiting
    if (!IsKeyboardOrMouseKey(Key)) return;

    StopCapture();
    BindKey(RowIndex, Key);
    PlayClickSound();
}

void UKeyBindingsWidget::BindKey(int32 RowIndex, const FKey& Key)
{
    UEnhancedInputUserSettings* Settings = GetInputSettings();
    const FPlayerKeyMapping* Mapping = Rows.IsValidIndex(RowIndex) ? FindKeyboardMapping(Settings, Rows[RowIndex].MappingName) : nullptr;
    if (!Mapping) return;

    const FKey PreviousKey = Mapping->GetCurrentKey();
    if (PreviousKey == Key) return;

    // The row which had the new key takes the previous one
    for (int32 i = 0; i < Rows.Num(); ++i)
    {
        const FPlayerKeyMapping* Other = i != RowIndex ? FindKeyboardMapping(Settings, Rows[i].MappingName) : nullptr;
        if (Other && Other->GetCurrentKey() == Key) MapKey(Settings, *Other, PreviousKey);
    }
    MapKey(Settings, *Mapping, Key);
    SaveBindings(Settings);
}

void UKeyBindingsWidget::SaveBindings(UEnhancedInputUserSettings* Settings)
{
    if (IsValid(Settings)) Settings->AsyncSaveSettings();
    RefreshRows();
}

void UKeyBindingsWidget::RefreshRows()
{
    const UEnhancedInputUserSettings* Settings = GetInputSettings();
    TArray<const FPlayerKeyMapping*> Mappings;
    Algo::Transform(Rows, Mappings, [&] (const FBindingRow& Row) { return FindKeyboardMapping(Settings, Row.MappingName); });

    TMap<FKey, int32> KeyUsages;
    for (const FPlayerKeyMapping* Mapping : Mappings)
        if (Mapping && Mapping->GetCurrentKey().IsValid()) ++KeyUsages.FindOrAdd(Mapping->GetCurrentKey());

    for (int32 i = 0; i < Rows.Num(); ++i)
    {
        const FBindingRow& Row = Rows[i];
        const FKey Key = Mappings[i] ? Mappings[i]->GetCurrentKey() : EKeys::Invalid;
        const bool bCapturing = i == CapturingRow;
        const bool bConflict = Key.IsValid() && KeyUsages.FindRef(Key) > 1;

        if (Row.KeyText.IsValid())
        {
            constexpr bool bLongDisplayName = false;
            const FText KeyName = Key.IsValid() ? Key.GetDisplayName(bLongDisplayName) : FText::FromString(TEXT("-"));
            Row.KeyText->SetText(bCapturing ? FText::FromString(TEXT("Press a key...")) : KeyName);
            Row.KeyText->SetColorAndOpacity(bCapturing ? CaptureColor : bConflict ? ConflictColor : FLinearColor::White);
        }

        const bool bCustomized = Mappings[i] && Mappings[i]->IsCustomized();
        if (Row.ResetButton.IsValid()) Row.ResetButton->SetIsEnabled(bCustomized);
        if (Row.ResetText.IsValid()) Row.ResetText->SetColorAndOpacity(bCustomized ? FLinearColor::White : DISABLED_RESET_COLOR);
    }

    const bool bAnyCustomized = Mappings.ContainsByPredicate([] (const FPlayerKeyMapping* Mapping) { return Mapping && Mapping->IsCustomized(); });
    if (IsValid(ResetAllButton)) ResetAllButton->SetIsEnabled(bAnyCustomized);
}

int32 UKeyBindingsWidget::FindActiveRow(bool bResetButton) const
{
    TArray<UButton*> Buttons;
    Algo::Transform(Rows, Buttons, [bResetButton] (const FBindingRow& Row) { return bResetButton ? Row.ResetButton.Get() : Row.Button.Get(); });
    constexpr bool bPreferHovered = true;
    return FindActiveButton(Buttons, bPreferHovered);
}

void UKeyBindingsWidget::PlayClickSound() const
{
    if (IsValid(BackButton) && FSlateApplication::IsInitialized())
        FSlateApplication::Get().PlaySound(BackButton->GetStyle().PressedSlateSound);
}
