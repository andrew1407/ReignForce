// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/ButtonGamepadPrompt.h"
#include "Blueprint/WidgetTree.h"

#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBoxSlot.h"

#include "UI/Common/MenuLayoutUtils.h"
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

    /** Padding of a box (vertical or horizontal) slot, false for other slots */
    bool TryGetBoxSlotPadding(UPanelSlot* Slot, FMargin& OutPadding)
    {
        if (auto VerticalSlot = Cast<UVerticalBoxSlot>(Slot)) OutPadding = VerticalSlot->GetPadding();
        else if (auto HorizontalSlot = Cast<UHorizontalBoxSlot>(Slot)) OutPadding = HorizontalSlot->GetPadding();
        else return false;
        return true;
    }

    void SetBoxSlotPadding(UPanelSlot* Slot, const FMargin& Padding)
    {
        if (auto VerticalSlot = Cast<UVerticalBoxSlot>(Slot)) VerticalSlot->SetPadding(Padding);
        else if (auto HorizontalSlot = Cast<UHorizontalBoxSlot>(Slot)) HorizontalSlot->SetPadding(Padding);
    }
}

void FButtonGamepadPrompt::Attach(UWidgetTree* WidgetTree, UButton* Button, const FMargin& InStylePadding)
{
    UWidget* Content = IsValid(Button) ? Button->GetContent() : nullptr;
    if (!IsValid(Content) || !IsValid(WidgetTree)) return;
    StylePadding = InStylePadding;

    // The button slot layout (padding, alignment) moves to the [prompt][content] row, the content keeps its own sizing in it
    auto ContentSlot = Cast<UButtonSlot>(Content->Slot);
    const FMargin ContentPadding = IsValid(ContentSlot) ? ContentSlot->GetPadding() : FMargin(0);
    const EHorizontalAlignment ContentHAlign = IsValid(ContentSlot) ? ContentSlot->GetHorizontalAlignment() : HAlign_Center;
    const EVerticalAlignment ContentVAlign = IsValid(ContentSlot) ? ContentSlot->GetVerticalAlignment() : VAlign_Center;

    // The prompt is about the label line height
    auto Label = Cast<UTextBlock>(Content);
    const float FontSize = IsValid(Label) ? Label->GetFont().Size : 0.f;
    Size = FontSize > 0 ? FMath::Clamp(FontSize * PROMPT_SIZE_PER_FONT_SIZE, PROMPT_MIN_SIZE, PROMPT_MAX_SIZE) : PROMPT_DEFAULT_SIZE;

    // Centered label: the [prompt][label] pair is centered as one (the label glides aside while the prompt grows), the button grows if needed
    const bool bCentered = IsValid(Label) && (ContentHAlign == HAlign_Center
        || (ContentHAlign == HAlign_Fill && MenuLayoutUtils::GetJustification(Label) == ETextJustify::Center));
    if (bCentered) CenteredLabel = Label;

    auto Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    Button->SetContent(Row);

    // The icon stays whole on a canvas (at the label side) while the clipped box around it grows: it slides out from behind the label
    auto Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
    Box->SetWidthOverride(0);
    Box->SetHeightOverride(Size);
    Box->SetClipping(EWidgetClipping::ClipToBounds);
    // Collapsed takes no space: the label stays where it was until the prompt shows up
    Box->SetVisibility(ESlateVisibility::Collapsed);
    auto Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
    Box->SetContent(Canvas);
    auto Icon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
    Icon->SetRenderTransformPivot(FVector2D(.5f));
    if (UCanvasPanelSlot* IconSlot = Canvas->AddChildToCanvas(Icon))
    {
        IconSlot->SetAnchors(FAnchors(1, .5f));
        IconSlot->SetAlignment(FVector2D(1, .5f));
        IconSlot->SetPosition(FVector2D(-PROMPT_GAP, 0));
        IconSlot->SetSize(FVector2D(Size));
    }
    if (UHorizontalBoxSlot* PromptSlot = Row->AddChildToHorizontalBox(Box))
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
        BaseSlotLayout = CanvasSlot->GetOffsets();
        // A fixed size canvas slot (not stretched between anchors) knows its width before the first layout
        const FAnchors Anchors = CanvasSlot->GetAnchors();
        bBaseWidthFromSlot = FMath::IsNearlyEqual(Anchors.Minimum.X, Anchors.Maximum.X) && !CanvasSlot->GetAutoSize();
        if (bBaseWidthFromSlot) BaseWidth = BaseSlotLayout.Right;
    }
    else
    {
        TryGetBoxSlotPadding(Button->Slot, BaseSlotLayout);
    }

    PromptBox = Box;
    GamepadIcon = Icon;
}

void FButtonGamepadPrompt::Update(UButton* Button, const FKey& Key, float DeltaTime)
{
    if (!GamepadIcon.IsValid()) return;
    if (Key.IsValid() && Key != PromptKey) ChangeKey(Key);

    const float Target = Key.IsValid() && Key == PromptKey ? 1.f : 0.f;
    const float PreviousProgress = Progress;
    const float PreviousPulse = Pulse;
    Progress = FMath::FInterpConstantTo(Progress, Target, DeltaTime, 1.f / PROMPT_ANIMATION_TIME);
    Pulse = FMath::FInterpConstantTo(Pulse, 0.f, DeltaTime, 1.f / PROMPT_PULSE_TIME);

    const bool bAnimating = Progress != PreviousProgress || Pulse != PreviousPulse;
    // The centered label room is kept up to date while the prompt is shown (the label text may change)
    if (bAnimating || Progress > 0 || CenteredLabel.IsValid()) ApplyProgress(Button);
}

FKey FButtonGamepadPrompt::GetShownKey() const
{
    return Progress > 0 ? PromptKey : EKeys::Invalid;
}

void FButtonGamepadPrompt::ChangeKey(const FKey& Key)
{
    FSlateBrush Brush;
    if (!UReignForceGamepadControllerData::GetGamepadKeyBrush(Key, Brush)) return;
    Brush.ImageSize = FVector2D(Size);
    GamepadIcon->SetBrush(Brush);
    // A shown prompt changing its key pops, so the change is noticed
    if (Progress > 0) Pulse = 1;
    PromptKey = Key;
}

void FButtonGamepadPrompt::ApplyProgress(UButton* Button)
{
    USizeBox* Box = PromptBox.Get();
    UImage* Icon = GamepadIcon.Get();
    if (!(IsValid(Box) && IsValid(Icon))) return;

    const ESlateVisibility PromptVisibility = Progress > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
    if (Box->GetVisibility() != PromptVisibility) Box->SetVisibility(PromptVisibility);

    const float Eased = FMath::InterpEaseInOut(0.f, 1.f, Progress, 2.f);
    const float FullWidth = Size + PROMPT_GAP;
    const float Width = FullWidth * Eased;
    if (!FMath::IsNearlyEqual(Box->GetWidthOverride(), Width, .01f)) Box->SetWidthOverride(Width);

    if (!FMath::IsNearlyEqual(Icon->GetRenderOpacity(), Progress)) Icon->SetRenderOpacity(Progress);
    // Slides from the label side & pops a bit when the key changes
    FWidgetTransform Transform;
    Transform.Translation = FVector2D(Size * PROMPT_SLIDE * (1 - Eased), 0);
    Transform.Scale = FVector2D(1 + PROMPT_PULSE_SCALE * FMath::Sin(Pulse * PI));
    const FWidgetTransform& Current = Icon->GetRenderTransform();
    if (!(Current.Translation.Equals(Transform.Translation, .01) && Current.Scale.Equals(Transform.Scale, .001))) Icon->SetRenderTransform(Transform);

    UTextBlock* Label = CenteredLabel.Get();
    if (!(IsValid(Label) && IsValid(Button))) return;

    // Room the centered [prompt][label] needs with the paddings & the same gap on the free side: the button grows (eased with the
    // prompt, so the width & the label move together) by what doesn't fit - a fixed size canvas slot grows freely, a box item takes its side paddings
    if (AppliedGrowth < .5f && !bBaseWidthFromSlot)
    {
        // The slot width isn't known up front: it's measured while the button has its own size
        const float GeometryWidth = Button->GetCachedGeometry().GetLocalSize().X;
        if (GeometryWidth > 0) BaseWidth = GeometryWidth;
    }
    auto RowSlot = IsValid(Button->GetContent()) ? Cast<UButtonSlot>(Button->GetContent()->Slot) : nullptr;
    const FMargin ContentPadding = IsValid(RowSlot) ? RowSlot->GetPadding() : FMargin(0);
    const float Needed = Label->GetDesiredSize().X + FullWidth + PROMPT_GAP + StylePadding.GetTotalSpaceAlong<Orient_Horizontal>() +
        ContentPadding.GetTotalSpaceAlong<Orient_Horizontal>();

    FMargin Unused;
    float MaxGrowth = 0;
    if (bBaseWidthFromSlot) MaxGrowth = TNumericLimits<float>::Max();
    else if (TryGetBoxSlotPadding(Button->Slot, Unused)) MaxGrowth = BaseSlotLayout.GetTotalSpaceAlong<Orient_Horizontal>();

    const float Missing = BaseWidth > 0 ? FMath::Max(0.f, Needed - BaseWidth) : 0.f;
    ApplyGrowth(Button, FMath::RoundToFloat(FMath::Min(Missing, MaxGrowth) * Eased));
}

void FButtonGamepadPrompt::ApplyGrowth(UButton* Button, float Growth)
{
    if (FMath::IsNearlyEqual(AppliedGrowth, Growth, .5f)) return;

    const FMargin& Base = BaseSlotLayout;
    if (auto CanvasSlot = Cast<UCanvasPanelSlot>(Button->Slot))
    {
        AppliedGrowth = Growth;
        // Wider by the growth around the same center: the position moves by its part on the alignment side
        FMargin Offsets = Base;
        Offsets.Left = Base.Left - Growth * (.5f - CanvasSlot->GetAlignment().X);
        Offsets.Right = Base.Right + Growth;
        CanvasSlot->SetOffsets(Offsets);
        return;
    }

    // Side paddings give the room evenly (the vertical ones may be changed by the menu layout, they're kept)
    FMargin SlotPadding;
    if (!TryGetBoxSlotPadding(Button->Slot, SlotPadding)) return;
    AppliedGrowth = Growth;
    // Half from each side, the other side gives the rest when one runs out
    const float RightShare = FMath::Min(Base.Right, Growth - FMath::Min(Base.Left, Growth * .5f));
    const float LeftShare = FMath::Min(Base.Left, Growth - RightShare);
    SlotPadding.Left = Base.Left - LeftShare;
    SlotPadding.Right = Base.Right - RightShare;
    SetBoxSlotPadding(Button->Slot, SlotPadding);
}
