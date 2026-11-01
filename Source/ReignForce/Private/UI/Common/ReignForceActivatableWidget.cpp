// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/ReignForceActivatableWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "Blueprint/WidgetTree.h"
#include "Styling/CoreStyle.h"

#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/PanelWidget.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/ContentWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "CommonTextBlock.h"
#include "CommonInputSubsystem.h"

#include "UI/Settings/GameSettingsWidget.h"
#include "UI/Common/ReignForceGamepadControllerData.h"


namespace
{
    /** Prompt icon size relative to the label font size (clamped), the default one for the buttons without a text label */
    constexpr float PROMPT_SIZE_PER_FONT_SIZE = 1.35f;
    constexpr float PROMPT_MIN_SIZE = 22;
    constexpr float PROMPT_MAX_SIZE = 32;
    constexpr float PROMPT_DEFAULT_SIZE = 28;
    constexpr float PROMPT_GAP = 10;
    /** Prompt slide in / out time (seconds) */
    constexpr float PROMPT_ANIMATION_TIME = .16f;
    constexpr float PROMPT_PULSE_TIME = .22f;
    constexpr float PROMPT_PULSE_SCALE = .18f;
    /** Prompt slides in from this fraction of its size towards the label */
    constexpr float PROMPT_SLIDE = .45f;

    /** Text alignment of a text block (the property has no getter, it is read by reflection) */
    ETextJustify::Type GetJustification(const UTextBlock* Text)
    {
        static const FProperty* Property = UTextLayoutWidget::StaticClass()->FindPropertyByName(TEXT("Justification"));
        const auto Value = Property ? Property->ContainerPtrToValuePtr<TEnumAsByte<ETextJustify::Type>>(Text) : nullptr;
        return Value ? Value->GetValue() : ETextJustify::Left;
    }

    /** The button text: its content, or the label next to the gamepad icon once the button is registered as focusable */
    UTextBlock* FindButtonLabel(const UButton* Button)
    {
        UWidget* Content = IsValid(Button) ? Button->GetContent() : nullptr;
        if (auto Label = Cast<UTextBlock>(Content)) return Label;
        auto Row = Cast<UHorizontalBox>(Content);
        if (!IsValid(Row)) return nullptr;
        for (UWidget* Child : Row->GetAllChildren())
            if (auto Label = Cast<UTextBlock>(Child)) return Label;
        return nullptr;
    }
}


UReignForceActivatableWidget::UReignForceActivatableWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bAutoActivate = true;
    bIsBackHandler = true;
    bRestoreLostFocus = true;

    SettingsMenuClass = UGameSettingsWidget::StaticClass();
}

void UReignForceActivatableWidget::RegisterFocusableButton(UButton* Button)
{
    if (!IsValid(Button)) return;
    bool bRegistered = FocusableButtons.ContainsByPredicate([&] (const FFocusableButtonState& State)
    {
        return State.Button == Button;
    });
    if (bRegistered) return;

    FFocusableButtonState& State = FocusableButtons.AddDefaulted_GetRef();
    State.Button = Button;
    State.DefaultStyle = Button->GetStyle();
    // Plain text tooltips only repeat the button labels in the engine default look
    Button->SetToolTipText(FText::GetEmpty());

    // The focus frame is the only highlight: the hovered brush (white in the menus) would also show on gamepad
    // focus, since the CommonUI virtual cursor hovers the focused button, and hide the white label
    FButtonStyle Style = State.DefaultStyle;
    Style.Hovered = Style.Normal;
    Button->SetStyle(Style);
    AddGamepadPrompt(State);

    Button->OnHovered.AddDynamic(this, &UReignForceActivatableWidget::HandleButtonHovered);
}

void UReignForceActivatableWidget::AddGamepadPrompt(FFocusableButtonState& State)
{
    UButton* Button = State.Button.Get();
    UWidget* Content = IsValid(Button) ? Button->GetContent() : nullptr;
    if (!IsValid(Content) || !IsValid(WidgetTree)) return;

    // The button slot layout (padding, alignment) moves to the [prompt][content] row, the content keeps its own sizing in it
    auto ContentSlot = Cast<UButtonSlot>(Content->Slot);
    const FMargin ContentPadding = IsValid(ContentSlot) ? ContentSlot->GetPadding() : FMargin(0);
    const EHorizontalAlignment ContentHAlign = IsValid(ContentSlot) ? ContentSlot->GetHorizontalAlignment() : HAlign_Center;
    const EVerticalAlignment ContentVAlign = IsValid(ContentSlot) ? ContentSlot->GetVerticalAlignment() : VAlign_Center;

    // The prompt is about the label line height
    auto Label = Cast<UTextBlock>(Content);
    const float FontSize = IsValid(Label) ? Label->GetFont().Size : 0.f;
    State.PromptSize = FontSize > 0 ? FMath::Clamp(FontSize * PROMPT_SIZE_PER_FONT_SIZE, PROMPT_MIN_SIZE, PROMPT_MAX_SIZE) : PROMPT_DEFAULT_SIZE;

    // Centered label: the [prompt][label] pair is centered as one (the label glides aside while the prompt grows), the button grows if needed
    const bool bCentered = IsValid(Label) && (ContentHAlign == HAlign_Center || (ContentHAlign == HAlign_Fill && GetJustification(Label) == ETextJustify::Center));
    if (bCentered) State.CenteredLabel = Label;

    auto Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    Button->SetContent(Row);

    // The icon stays whole on a canvas (at the label side) while the clipped box around it grows: it slides out from behind the label
    auto PromptBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
    PromptBox->SetWidthOverride(0);
    PromptBox->SetHeightOverride(State.PromptSize);
    PromptBox->SetClipping(EWidgetClipping::ClipToBounds);
    // Collapsed takes no space: the label stays where it was until the prompt shows up
    PromptBox->SetVisibility(ESlateVisibility::Collapsed);
    auto PromptCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
    PromptBox->SetContent(PromptCanvas);
    auto Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
    Icon->SetRenderTransformPivot(FVector2D(.5f));
    if (UCanvasPanelSlot* IconSlot = PromptCanvas->AddChildToCanvas(Icon))
    {
        IconSlot->SetAnchors(FAnchors(1, .5f));
        IconSlot->SetAlignment(FVector2D(1, .5f));
        IconSlot->SetPosition(FVector2D(-PROMPT_GAP, 0));
        IconSlot->SetSize(FVector2D(State.PromptSize));
    }
    if (UHorizontalBoxSlot* PromptSlot = Row->AddChildToHorizontalBox(PromptBox))
        PromptSlot->SetVerticalAlignment(VAlign_Center);

    if (UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(Content))
    {
        LabelSlot->SetVerticalAlignment(VAlign_Center);
        // A centered label sizes to its text, so the whole row (not only the label in the space left after the prompt) is centered
        if (ContentHAlign == HAlign_Fill && !bCentered) LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    }

    if (auto RowSlot = Cast<UButtonSlot>(Row->Slot))
    {
        RowSlot->SetPadding(ContentPadding);
        RowSlot->SetHorizontalAlignment(bCentered ? HAlign_Center : ContentHAlign);
        RowSlot->SetVerticalAlignment(ContentVAlign);
    }

    if (auto CanvasSlot = Cast<UCanvasPanelSlot>(Button->Slot))
    {
        State.BaseSlotLayout = CanvasSlot->GetOffsets();
        // A fixed size canvas slot (not stretched between anchors) knows its width before the first layout
        const FAnchors Anchors = CanvasSlot->GetAnchors();
        State.bBaseWidthFromSlot = FMath::IsNearlyEqual(Anchors.Minimum.X, Anchors.Maximum.X) && !CanvasSlot->GetAutoSize();
        if (State.bBaseWidthFromSlot) State.BaseWidth = State.BaseSlotLayout.Right;
    }
    else if (auto BoxSlot = Cast<UVerticalBoxSlot>(Button->Slot))
    {
        State.BaseSlotLayout = BoxSlot->GetPadding();
    }
    else if (auto RowBoxSlot = Cast<UHorizontalBoxSlot>(Button->Slot))
    {
        State.BaseSlotLayout = RowBoxSlot->GetPadding();
    }

    State.PromptBox = PromptBox;
    State.GamepadIcon = Icon;
}

void UReignForceActivatableWidget::ApplyPromptProgress(FFocusableButtonState& State)
{
    USizeBox* PromptBox = State.PromptBox.Get();
    UImage* Icon = State.GamepadIcon.Get();
    if (!(IsValid(PromptBox) && IsValid(Icon))) return;

    const bool bShown = State.PromptProgress > 0;
    const ESlateVisibility PromptVisibility = bShown ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
    if (PromptBox->GetVisibility() != PromptVisibility) PromptBox->SetVisibility(PromptVisibility);

    const float Eased = FMath::InterpEaseInOut(0.f, 1.f, State.PromptProgress, 2.f);
    const float FullWidth = State.PromptSize + PROMPT_GAP;
    const float Width = FullWidth * Eased;
    if (!FMath::IsNearlyEqual(PromptBox->GetWidthOverride(), Width, .01f)) PromptBox->SetWidthOverride(Width);

    if (!FMath::IsNearlyEqual(Icon->GetRenderOpacity(), State.PromptProgress)) Icon->SetRenderOpacity(State.PromptProgress);
    // Slides from the label side & pops a bit when the key changes
    const float Scale = 1 + PROMPT_PULSE_SCALE * FMath::Sin(State.PromptPulse * PI);
    FWidgetTransform Transform;
    Transform.Translation = FVector2D(State.PromptSize * PROMPT_SLIDE * (1 - Eased), 0);
    Transform.Scale = FVector2D(Scale);
    const FWidgetTransform& Current = Icon->GetRenderTransform();
    if (!(Current.Translation.Equals(Transform.Translation, .01) && Current.Scale.Equals(Transform.Scale, .001))) Icon->SetRenderTransform(Transform);

    UTextBlock* Label = State.CenteredLabel.Get();
    UButton* Button = State.Button.Get();
    if (!(IsValid(Label) && IsValid(Button))) return;

    // Room the centered [prompt][label] needs with the paddings & the same gap on the free side: the button grows (eased with the
    // prompt, so the width & the label move together) by what doesn't fit - a fixed size canvas slot grows freely, a box item takes its side paddings
    const bool bNoGrowth = State.AppliedGrowth < .5f;
    if (bNoGrowth && !State.bBaseWidthFromSlot)
    {
        // The slot width isn't known up front: it's measured while the button has its own size
        const float GeometryWidth = Button->GetCachedGeometry().GetLocalSize().X;
        if (GeometryWidth > 0) State.BaseWidth = GeometryWidth;
    }
    const FMargin StylePadding = State.DefaultStyle.NormalPadding;
    auto RowSlot = IsValid(Button->GetContent()) ? Cast<UButtonSlot>(Button->GetContent()->Slot) : nullptr;
    const FMargin ContentPadding = IsValid(RowSlot) ? RowSlot->GetPadding() : FMargin(0);
    const float Needed = Label->GetDesiredSize().X + FullWidth + PROMPT_GAP + StylePadding.GetTotalSpaceAlong<Orient_Horizontal>() +
        ContentPadding.GetTotalSpaceAlong<Orient_Horizontal>();

    float MaxGrowth = 0;
    if (State.bBaseWidthFromSlot) MaxGrowth = TNumericLimits<float>::Max();
    else if (Cast<UVerticalBoxSlot>(Button->Slot) || Cast<UHorizontalBoxSlot>(Button->Slot)) MaxGrowth = State.BaseSlotLayout.GetTotalSpaceAlong<Orient_Horizontal>();

    const float Missing = State.BaseWidth > 0 ? FMath::Max(0.f, Needed - State.BaseWidth) : 0.f;
    const float Growth = FMath::Min(Missing, MaxGrowth);
    ApplyButtonGrowth(State, FMath::RoundToFloat(Growth * Eased));
}

void UReignForceActivatableWidget::ApplyButtonGrowth(FFocusableButtonState& State, float Growth)
{
    UButton* Button = State.Button.Get();
    if (!IsValid(Button) || FMath::IsNearlyEqual(State.AppliedGrowth, Growth, .5f)) return;
    State.AppliedGrowth = Growth;

    const FMargin& Base = State.BaseSlotLayout;
    if (auto CanvasSlot = Cast<UCanvasPanelSlot>(Button->Slot))
    {
        // Wider by the growth around the same center: the position moves by its part on the alignment side
        const float AlignmentX = CanvasSlot->GetAlignment().X;
        FMargin Offsets = Base;
        Offsets.Left = Base.Left - Growth * (.5f - AlignmentX);
        Offsets.Right = Base.Right + Growth;
        CanvasSlot->SetOffsets(Offsets);
    }
    else
    {
        auto VerticalSlot = Cast<UVerticalBoxSlot>(Button->Slot);
        auto HorizontalSlot = Cast<UHorizontalBoxSlot>(Button->Slot);
        if (!(VerticalSlot || HorizontalSlot)) return;
        // Side paddings give the room evenly (the vertical ones may be changed by the menu layout, they're kept)
        FMargin SlotPadding = VerticalSlot ? VerticalSlot->GetPadding() : HorizontalSlot->GetPadding();
        // Half from each side, the other side gives the rest when one runs out
        const float RightShare = FMath::Min(Base.Right, Growth - FMath::Min(Base.Left, Growth * .5f));
        const float LeftShare = FMath::Min(Base.Left, Growth - RightShare);
        SlotPadding.Left = Base.Left - LeftShare;
        SlotPadding.Right = Base.Right - RightShare;
        if (VerticalSlot) VerticalSlot->SetPadding(SlotPadding);
        else HorizontalSlot->SetPadding(SlotPadding);
    }
}

void UReignForceActivatableWidget::SetBackActionButton(UButton* Button)
{
    BackActionButton = Button;
}

FKey UReignForceActivatableWidget::GetGamepadPromptKey(const UButton* Button, bool bFocused) const
{
    // Prompts of a menu covered by a submenu or a modal would show the keys which act on the menu over it
    if (!(IsActivated() && IsButtonAvailable(Button))) return EKeys::Invalid;
    if (bFocused) return EKeys::Gamepad_FaceButton_Bottom;
    if (Button == BackActionButton.Get()) return EKeys::Gamepad_FaceButton_Right;
    return EKeys::Invalid;
}

FKey UReignForceActivatableWidget::GetShownGamepadPrompt(const UButton* Button) const
{
    const FFocusableButtonState* State = FocusableButtons.FindByPredicate([&] (const FFocusableButtonState& Item) { return Item.Button == Button; });
    return State && State->PromptProgress > 0 ? State->PromptKey : EKeys::Invalid;
}

bool UReignForceActivatableWidget::IsGamepadUsed() const
{
    const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
    return InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;
}

void UReignForceActivatableWidget::FocusButton(UButton* Button)
{
    if (!IsButtonAvailable(Button)) return;
    LastFocusedButton = Button;
    Button->SetFocus();
}

void UReignForceActivatableWidget::RequestClose()
{
    OnCloseRequested.Broadcast();
}

void UReignForceActivatableWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    UpdateButtonsHighlight(InDeltaTime);

    // The menu is fullscreen: its height is the screen one in the layout units
    if (IsValid(ScreenHeightLimitedBox))
    {
        constexpr float MinHeight = 120;
        const float Available = FMath::Max(MinHeight, MyGeometry.GetLocalSize().Y - ScreenHeightMargin * 2);
        if (!FMath::IsNearlyEqual(ScreenHeightLimitedBox->GetMaxDesiredHeight(), Available, 1.f))
            ScreenHeightLimitedBox->SetMaxDesiredHeight(Available);
    }

    // Clicking an empty space or switching the input mode drops the focus, so keyboard & gamepad navigation would stop working
    bool bFocusLost = bRestoreLostFocus && IsActivated() && !HasAnyUserFocus() && !HasFocusedDescendants();
    if (!bFocusLost) return;
    UWidget* Target = GetDesiredFocusTarget();
    if (IsValid(Target)) Target->SetFocus();
}

int32 UReignForceActivatableWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    int32 MaxLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
    // Children are painted already, so the focused button geometry is up to date
    for (const FFocusableButtonState& State : FocusableButtons)
    {
        if (!State.bHighlighted) continue;
        const int32 OutlineLayer = FocusOutline::Paint(State.Button.Get(), FocusOutlineStyle, OutDrawElements, MaxLayer);
        MaxLayer = FMath::Max(MaxLayer, OutlineLayer);
    }
    return MaxLayer;
}

void UReignForceActivatableWidget::NativeDestruct()
{
    CloseChildMenu();
    OnCloseRequested.Clear();

    Super::NativeDestruct();
}

UWidget* UReignForceActivatableWidget::NativeGetDesiredFocusTarget() const
{
    if (IsButtonAvailable(LastFocusedButton.Get())) return LastFocusedButton.Get();
    for (const FFocusableButtonState& State : FocusableButtons)
    {
        if (IsButtonAvailable(State.Button.Get())) return State.Button.Get();
    }
    return Super::NativeGetDesiredFocusTarget();
}

bool UReignForceActivatableWidget::NativeOnHandleBackAction()
{
    return HandleBackAction();
}

bool UReignForceActivatableWidget::HandleBackAction()
{
    if (!OnCloseRequested.IsBound()) return false;
    RequestClose();
    return true;
}

UButton* UReignForceActivatableWidget::CreateButtonLike(UButton* Template, const FText& Label, FName Name, int32 IndexOffset)
{
    if (!(IsValid(Template) && IsValid(WidgetTree))) return nullptr;
    UPanelWidget* Parent = Template->GetParent();
    if (!IsValid(Parent)) return nullptr;

    auto Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
    Button->SetStyle(Template->GetStyle());
    Button->SetColorAndOpacity(Template->GetColorAndOpacity());
    Button->SetBackgroundColor(Template->GetBackgroundColor());

    UTextBlock* TemplateLabel = FindButtonLabel(Template);
    auto Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    Text->SetText(Label);
    if (IsValid(TemplateLabel))
    {
        Text->SetFont(TemplateLabel->GetFont());
        Text->SetColorAndOpacity(TemplateLabel->GetColorAndOpacity());
        Text->SetShadowOffset(TemplateLabel->GetShadowOffset());
        Text->SetShadowColorAndOpacity(TemplateLabel->GetShadowColorAndOpacity());
        // The template content slot (its button padding & alignment), also when it's the gamepad icon row already
        Button->AddChild(Text, Template->GetContent()->Slot);
    }
    else
    {
        Button->AddChild(Text);
    }

    const int32 Index = FMath::Clamp(Parent->GetChildIndex(Template) + IndexOffset, 0, Parent->GetChildrenCount());
    // Menu items are separated by spacers, so the new button gets its own one on the side the neighbour spacer is
    UWidget* Previous = Index > 0 ? Parent->GetChildAt(Index - 1) : nullptr;
    UWidget* Next = Index < Parent->GetChildrenCount() ? Parent->GetChildAt(Index) : nullptr;
    auto PreviousSpacer = Cast<USpacer>(Previous);
    auto NextSpacer = Cast<USpacer>(Next);
    Parent->InsertChildAt(Index, Button, Template->Slot);

    USpacer* SpacerTemplate = IsValid(PreviousSpacer) ? PreviousSpacer : NextSpacer;
    if (!IsValid(SpacerTemplate)) return Button;
    auto Spacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass());
    Spacer->SetSize(SpacerTemplate->GetSize());
    // [spacer, new, template] -> [spacer, new, spacer, template]; [template, new, spacer] -> [template, spacer, new, spacer]
    const int32 SpacerIndex = IsValid(PreviousSpacer) ? Index + 1 : Index;
    Parent->InsertChildAt(SpacerIndex, Spacer, SpacerTemplate->Slot);
    return Button;
}

void UReignForceActivatableWidget::SetMenuItemsGap(UWidget* Item, float Gap)
{
    UPanelWidget* Panel = IsValid(Item) ? Item->GetParent() : nullptr;
    if (!IsValid(Panel)) return;

    // Items spread over the whole column height (fill) would ignore the gaps, so each takes only its own height
    const TArray<UWidget*> Children = Panel->GetAllChildren();
    bool bHasSpacers = Children.ContainsByPredicate([] (const UWidget* Child) { return Child->IsA<USpacer>(); });
    for (UWidget* Child : Children)
    {
        auto ItemSlot = Cast<UVerticalBoxSlot>(Child->Slot);
        if (!IsValid(ItemSlot)) continue;
        ItemSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
        FMargin ItemPadding = ItemSlot->GetPadding();
        if (auto Spacer = Cast<USpacer>(Child))
        {
            Spacer->SetSize(FVector2D(Spacer->GetSize().X, Gap));
            ItemPadding.Top = ItemPadding.Bottom = 0;
        }
        else
        {
            // Without spacers the gap is made of the items paddings
            ItemPadding.Top = ItemPadding.Bottom = bHasSpacers ? 0 : Gap * .5f;
        }
        ItemSlot->SetPadding(ItemPadding);
    }
}

USizeBox* UReignForceActivatableWidget::WrapInScrollBox(UWidget* Content, UScrollBox** OutScrollBox)
{
    if (!(IsValid(Content) && IsValid(WidgetTree))) return nullptr;

    auto Limit = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
    auto Scroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass());
    // Keyboard & gamepad navigation reaches the items out of the view and scrolls them in
    Scroll->SetScrollWhenFocusChanges(EScrollWhenFocusChanges::AnimatedScroll);
    Scroll->SetNavigationDestination(EDescendantScrollDestination::IntoView);
    Scroll->SetAnimateWheelScrolling(true);

    // The size box takes the content place with the same slot settings (layout on the parent stays the same)
    if (UPanelWidget* Parent = Content->GetParent())
    {
        if (auto SingleChildParent = Cast<UContentWidget>(Parent))
        {
            SingleChildParent->SetContent(Limit);
        }
        else
        {
            Parent->InsertChildAt(Parent->GetChildIndex(Content), Limit, Content->Slot);
            Parent->RemoveChild(Content);
        }
    }
    else if (WidgetTree->RootWidget == Content)
    {
        WidgetTree->RootWidget = Limit;
    }
    Limit->SetContent(Scroll);
    // Focus frames are drawn outside the items & clipped by the scroll box, the inset keeps the edge ones whole
    if (auto ContentSlot = Cast<UScrollBoxSlot>(Scroll->AddChild(Content)))
        ContentSlot->SetPadding(FMargin(SCROLLED_CONTENT_INSET));

    if (OutScrollBox) *OutScrollBox = Scroll;
    return Limit;
}

void UReignForceActivatableWidget::LimitHeightToScreen(USizeBox* Box, float ScreenMargin)
{
    ScreenHeightLimitedBox = Box;
    ScreenHeightMargin = ScreenMargin;
}

void UReignForceActivatableWidget::MakeMenuItemsScrollableCentered(UWidget* Item, float ScreenMargin)
{
    UPanelWidget* Panel = IsValid(Item) ? Item->GetParent() : nullptr;
    auto PanelSlot = IsValid(Panel) ? Cast<UCanvasPanelSlot>(Panel->Slot) : nullptr;
    if (!IsValid(PanelSlot)) return;

    const FAnchors Anchors = PanelSlot->GetAnchors();
    const FMargin Offsets = PanelSlot->GetOffsets();
    bool bStretchedX = !FMath::IsNearlyEqual(Anchors.Minimum.X, Anchors.Maximum.X);

    USizeBox* Limit = WrapInScrollBox(Panel);
    auto LimitSlot = IsValid(Limit) ? Cast<UCanvasPanelSlot>(Limit->Slot) : nullptr;
    if (!IsValid(LimitSlot)) return;

    // The column takes its items height (up to the screen one) & stays in the screen middle; the width stays as designed
    if (!bStretchedX) Limit->SetWidthOverride(Offsets.Right + SCROLLED_CONTENT_INSET * 2);
    LimitSlot->SetAnchors(FAnchors(Anchors.Minimum.X, .5f, Anchors.Maximum.X, .5f));
    LimitSlot->SetAlignment(FVector2D(LimitSlot->GetAlignment().X, .5f));
    LimitSlot->SetAutoSize(true);
    LimitSlot->SetOffsets(FMargin(Offsets.Left, 0, bStretchedX ? Offsets.Right : 0, 0));
    LimitHeightToScreen(Limit, ScreenMargin);
}

FSlateFontInfo UReignForceActivatableWidget::MakeFont(const UButton* StyleTemplate, int32 Size)
{
    UTextBlock* TemplateLabel = FindButtonLabel(StyleTemplate);
    FSlateFontInfo Font = IsValid(TemplateLabel) ? TemplateLabel->GetFont() : FCoreStyle::GetDefaultFontStyle("Bold", Size);
    Font.Size = Size;
    return Font;
}

UButton* UReignForceActivatableWidget::MakeButton(UButton* StyleTemplate, UWidget* Content, FName Name)
{
    auto Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
    if (IsValid(StyleTemplate))
    {
        Button->SetStyle(StyleTemplate->GetStyle());
        Button->SetColorAndOpacity(StyleTemplate->GetColorAndOpacity());
        Button->SetBackgroundColor(StyleTemplate->GetBackgroundColor());
    }
    if (UButtonSlot* ContentSlot = Cast<UButtonSlot>(Button->AddChild(Content)))
    {
        ContentSlot->SetPadding(FMargin(18, 6));
        ContentSlot->SetHorizontalAlignment(Content->IsA<UTextBlock>() ? HAlign_Center : HAlign_Fill);
        ContentSlot->SetVerticalAlignment(VAlign_Center);
    }
    return Button;
}

UTextBlock* UReignForceActivatableWidget::MakeText(const FText& Text, const FSlateFontInfo& Font, const FSlateColor& Color)
{
    auto TextBlock = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass());
    TextBlock->SetText(Text);
    TextBlock->SetFont(Font);
    TextBlock->SetColorAndOpacity(Color);
    TextBlock->SetShadowOffset(FVector2D(1));
    TextBlock->SetShadowColorAndOpacity(FLinearColor(0, 0, 0, .6f));
    return TextBlock;
}

UVerticalBox* UReignForceActivatableWidget::ConstructMenuColumn(const FText& Title, const FSlateFontInfo& TitleFont, float Width, const FLinearColor& Background)
{
    auto BackgroundBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
    BackgroundBorder->SetBrushColor(Background);
    BackgroundBorder->SetHorizontalAlignment(HAlign_Center);
    BackgroundBorder->SetVerticalAlignment(VAlign_Center);
    WidgetTree->RootWidget = BackgroundBorder;

    auto Frame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Frame"));
    Frame->SetWidthOverride(Width);
    BackgroundBorder->SetContent(Frame);

    auto Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
    Frame->SetContent(Column);

    UTextBlock* TitleText = MakeText(Title, TitleFont, FLinearColor::White);
    if (UVerticalBoxSlot* TitleSlot = Column->AddChildToVerticalBox(TitleText))
    {
        TitleSlot->SetHorizontalAlignment(HAlign_Center);
        TitleSlot->SetPadding(FMargin(0, 0, 0, 24));
    }
    return Column;
}

void UReignForceActivatableWidget::NativeOnActivated()
{
    Super::NativeOnActivated();
    if (bNavigationBlocked)
    {
        SetVisibility(VisibilityBeforeDeactivation);
        bNavigationBlocked = false;
    }
    // A button under the resting cursor is hovered as soon as the menu shows up, it shouldn't take the focus
    if (FSlateApplication::IsInitialized()) HoverCursorPosition = FSlateApplication::Get().GetCursorPos();
}

void UReignForceActivatableWidget::NativeOnDeactivated()
{
    Super::NativeOnDeactivated();
    // A menu covered by a submenu or modal is still on screen: its buttons would be found by the arrows & gamepad
    // navigation of the menu over it (it searches by position) and the focus would bounce back
    const ESlateVisibility Current = GetVisibility();
    bool bCanBlock = !bNavigationBlocked && Current != ESlateVisibility::Collapsed && Current != ESlateVisibility::Hidden;
    if (!bCanBlock) return;
    VisibilityBeforeDeactivation = Current;
    bNavigationBlocked = true;
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

bool UReignForceActivatableWidget::IsHoverFromMouseMove()
{
    if (!FSlateApplication::IsInitialized()) return false;
    // Gamepad virtual cursor hovers the buttons too, only the real mouse moves the focus
    auto InputSubsystem = UCommonInputSubsystem::Get(GetOwningLocalPlayer());
    if (InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad) return false;

    const FVector2D CursorPosition = FSlateApplication::Get().GetCursorPos();
    if (CursorPosition.Equals(HoverCursorPosition, 1.)) return false;
    HoverCursorPosition = CursorPosition;
    return true;
}

void UReignForceActivatableWidget::HandleButtonHovered()
{
    if (!IsActivated() || !IsHoverFromMouseMove()) return;
    for (const FFocusableButtonState& State : FocusableButtons)
    {
        UButton* Button = State.Button.Get();
        bool bHoveredByMouse = IsButtonAvailable(Button) && Button->IsHovered();
        if (bHoveredByMouse && !Button->HasAnyUserFocus()) return FocusButton(Button);
    }
}

void UReignForceActivatableWidget::UpdateButtonsHighlight(float DeltaTime)
{
    const bool bGamepad = IsGamepadUsed();
    for (FFocusableButtonState& State : FocusableButtons)
    {
        UButton* Button = State.Button.Get();
        if (!IsValid(Button)) continue;
        bool bFocused = Button->HasAnyUserFocus() && Button->GetIsEnabled();

        // Gamepad prompts mark what the buttons do: "(A) Apply" on the focused one, "(B) Back" on the back action one
        if (UImage* Icon = State.GamepadIcon.Get())
        {
            const FKey Key = bGamepad ? GetGamepadPromptKey(Button, bFocused) : EKeys::Invalid;
            if (Key.IsValid() && Key != State.PromptKey)
            {
                FSlateBrush Brush;
                if (UReignForceGamepadControllerData::GetGamepadKeyBrush(Key, Brush))
                {
                    Brush.ImageSize = FVector2D(State.PromptSize);
                    Icon->SetBrush(Brush);
                    // A shown prompt changing its key pops, so the change is noticed
                    if (State.PromptProgress > 0) State.PromptPulse = 1;
                    State.PromptKey = Key;
                }
            }
            const float Target = Key.IsValid() && Key == State.PromptKey ? 1.f : 0.f;
            const float PreviousProgress = State.PromptProgress;
            State.PromptProgress = FMath::FInterpConstantTo(State.PromptProgress, Target, DeltaTime, 1.f / PROMPT_ANIMATION_TIME);
            const float PreviousPulse = State.PromptPulse;
            State.PromptPulse = FMath::FInterpConstantTo(State.PromptPulse, 0.f, DeltaTime, 1.f / PROMPT_PULSE_TIME);

            bool bAnimating = State.PromptProgress != PreviousProgress || State.PromptPulse != PreviousPulse;
            // The centered label room is kept up to date while the prompt is shown (the label text may change)
            if (bAnimating || State.PromptProgress > 0 || State.CenteredLabel.IsValid()) ApplyPromptProgress(State);
        }
        if (bFocused == State.bHighlighted) continue;

        if (bFocused)
        {
            LastFocusedButton = Button;
            // Hover sound is played by the button itself, keyboard & gamepad navigation needs it too
            if (!Button->IsHovered() && FSlateApplication::IsInitialized())
                FSlateApplication::Get().PlaySound(State.DefaultStyle.HoveredSlateSound);
        }
        State.bHighlighted = bFocused;
    }
}

bool UReignForceActivatableWidget::IsButtonAvailable(const UButton* Button) const
{
    return IsValid(Button) && Button->GetIsEnabled() && Button->IsVisible();
}

void UReignForceActivatableWidget::PushChildMenu(UReignForceActivatableWidget* Child, int32 ZOrder)
{
    if (!IsValid(Child)) return;
    CloseChildMenu();
    // The button which opened the submenu gets the focus back when it is closed
    ChildMenuOpener = LastFocusedButton;
    ChildMenu = Child;
    ChildMenu->OnCloseRequested.AddDynamic(this, &UReignForceActivatableWidget::CloseChildMenu);
    ChildMenu->AddToViewport(ZOrder);
    DeactivateWidget();
}

void UReignForceActivatableWidget::CloseChildMenu()
{
    if (!IsValid(ChildMenu)) return;
    ChildMenu->OnCloseRequested.RemoveDynamic(this, &UReignForceActivatableWidget::CloseChildMenu);
    ChildMenu->RemoveFromParent();
    ChildMenu = nullptr;
    if (IsInViewport() && !IsActivated()) ActivateWidget();
    if (IsButtonAvailable(ChildMenuOpener.Get())) FocusButton(ChildMenuOpener.Get());
    ChildMenuOpener = nullptr;
}

void UReignForceActivatableWidget::OpenSettingsMenu(UButton* StyleTemplate)
{
    if (!IsValid(SettingsMenuClass)) return;
    auto Settings = CreateWidget<UGameSettingsWidget>(GetOwningPlayer(), SettingsMenuClass);
    if (!IsValid(Settings)) return;
    Settings->Setup(StyleTemplate);
    PushChildMenu(Settings);
}
