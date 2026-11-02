// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/MenuLayoutUtils.h"
#include "Blueprint/WidgetTree.h"
#include "Styling/CoreStyle.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/ContentWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/PanelWidget.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "CommonTextBlock.h"


namespace
{
    const FMargin BUTTON_CONTENT_PADDING(18, 6);
    const FVector2D TEXT_SHADOW_OFFSET(1);
    const FLinearColor TEXT_SHADOW_COLOR(0, 0, 0, .6f);
    constexpr float TITLE_GAP = 24;
}

FSlateFontInfo MenuLayoutUtils::MakeFont(const UButton* StyleTemplate, int32 Size)
{
    UTextBlock* TemplateLabel = FindButtonLabel(StyleTemplate);
    FSlateFontInfo Font = IsValid(TemplateLabel) ? TemplateLabel->GetFont() : FCoreStyle::GetDefaultFontStyle("Bold", Size);
    Font.Size = Size;
    return Font;
}

UTextBlock* MenuLayoutUtils::MakeText(UWidgetTree* WidgetTree, const FText& Text, const FSlateFontInfo& Font, const FSlateColor& Color, TSubclassOf<UTextBlock> TextClass)
{
    auto TextBlock = WidgetTree->ConstructWidget<UTextBlock>(TextClass ? *TextClass : UCommonTextBlock::StaticClass());
    TextBlock->SetText(Text);
    TextBlock->SetFont(Font);
    TextBlock->SetColorAndOpacity(Color);
    TextBlock->SetShadowOffset(TEXT_SHADOW_OFFSET);
    TextBlock->SetShadowColorAndOpacity(TEXT_SHADOW_COLOR);
    return TextBlock;
}

UButton* MenuLayoutUtils::MakeButton(UWidgetTree* WidgetTree, UButton* StyleTemplate, UWidget* Content, FName Name)
{
    auto Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
    if (IsValid(StyleTemplate))
    {
        Button->SetStyle(StyleTemplate->GetStyle());
        Button->SetColorAndOpacity(StyleTemplate->GetColorAndOpacity());
        Button->SetBackgroundColor(StyleTemplate->GetBackgroundColor());
    }
    if (auto ContentSlot = Cast<UButtonSlot>(Button->AddChild(Content)))
    {
        ContentSlot->SetPadding(BUTTON_CONTENT_PADDING);
        ContentSlot->SetHorizontalAlignment(Content->IsA<UTextBlock>() ? HAlign_Center : HAlign_Fill);
        ContentSlot->SetVerticalAlignment(VAlign_Center);
    }
    return Button;
}

UButton* MenuLayoutUtils::CreateButtonLike(UWidgetTree* WidgetTree, UButton* Template, const FText& Label, FName Name, int32 IndexOffset)
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
        // The template content slot (its button padding & alignment), also when it's the gamepad prompt row already
        Button->AddChild(Text, Template->GetContent()->Slot);
    }
    else
    {
        Button->AddChild(Text);
    }

    const int32 Index = FMath::Clamp(Parent->GetChildIndex(Template) + IndexOffset, 0, Parent->GetChildrenCount());
    // Menu items are separated by spacers, so the new button gets its own one on the side the neighbour spacer is
    auto PreviousSpacer = Index > 0 ? Cast<USpacer>(Parent->GetChildAt(Index - 1)) : nullptr;
    auto NextSpacer = Index < Parent->GetChildrenCount() ? Cast<USpacer>(Parent->GetChildAt(Index)) : nullptr;
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

UVerticalBox* MenuLayoutUtils::ConstructMenuColumn(UWidgetTree* WidgetTree, const FText& Title, const FSlateFontInfo& TitleFont, float Width, const FLinearColor& Background)
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

    if (UVerticalBoxSlot* TitleSlot = Column->AddChildToVerticalBox(MakeText(WidgetTree, Title, TitleFont, FLinearColor::White)))
    {
        TitleSlot->SetHorizontalAlignment(HAlign_Center);
        TitleSlot->SetPadding(FMargin(0, 0, 0, TITLE_GAP));
    }
    return Column;
}

FSlateBrush MenuLayoutUtils::MakeRoundedBrush(const FLinearColor& Fill, const FLinearColor& Outline, float Radius, float OutlineWidth)
{
    FSlateBrush Brush;
    Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
    Brush.TintColor = Fill;
    Brush.OutlineSettings = FSlateBrushOutlineSettings(FVector4(Radius, Radius, Radius, Radius), FSlateColor(Outline), OutlineWidth);
    Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
    return Brush;
}

UTextBlock* MenuLayoutUtils::FindButtonLabel(const UButton* Button)
{
    UWidget* Content = IsValid(Button) ? Button->GetContent() : nullptr;
    if (auto Label = Cast<UTextBlock>(Content)) return Label;
    auto Row = Cast<UHorizontalBox>(Content);
    if (!IsValid(Row)) return nullptr;
    for (UWidget* Child : Row->GetAllChildren())
        if (auto Label = Cast<UTextBlock>(Child)) return Label;
    return nullptr;
}

ETextJustify::Type MenuLayoutUtils::GetJustification(const UTextBlock* Text)
{
    static const FProperty* Property = UTextLayoutWidget::StaticClass()->FindPropertyByName(TEXT("Justification"));
    const auto Value = Property && IsValid(Text) ? Property->ContainerPtrToValuePtr<TEnumAsByte<ETextJustify::Type>>(Text) : nullptr;
    return Value ? Value->GetValue() : ETextJustify::Left;
}

void MenuLayoutUtils::SetMenuItemsGap(UWidget* Item, float Gap)
{
    UPanelWidget* Panel = IsValid(Item) ? Item->GetParent() : nullptr;
    if (!IsValid(Panel)) return;

    // Items spread over the whole column height (fill) would ignore the gaps, so each takes only its own height
    const TArray<UWidget*> Children = Panel->GetAllChildren();
    const bool bHasSpacers = Children.ContainsByPredicate([] (const UWidget* Child) { return Child->IsA<USpacer>(); });
    for (UWidget* Child : Children)
    {
        auto ItemSlot = Cast<UVerticalBoxSlot>(Child->Slot);
        if (!IsValid(ItemSlot)) continue;
        ItemSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
        FMargin ItemPadding = ItemSlot->GetPadding();
        auto Spacer = Cast<USpacer>(Child);
        if (IsValid(Spacer)) Spacer->SetSize(FVector2D(Spacer->GetSize().X, Gap));
        // Without spacers the gap is made of the items paddings
        ItemPadding.Top = ItemPadding.Bottom = IsValid(Spacer) || bHasSpacers ? 0 : Gap * .5f;
        ItemSlot->SetPadding(ItemPadding);
    }
}

USizeBox* MenuLayoutUtils::WrapInScrollBox(UWidgetTree* WidgetTree, UWidget* Content)
{
    if (!(IsValid(Content) && IsValid(WidgetTree))) return nullptr;

    auto Limit = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
    auto Scroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass());
    // Keyboard & gamepad navigation reaches the items out of the view and scrolls them in
    Scroll->SetScrollWhenFocusChanges(EScrollWhenFocusChanges::AnimatedScroll);
    Scroll->SetNavigationDestination(EDescendantScrollDestination::IntoView);
    Scroll->SetAnimateWheelScrolling(true);

    // The size box takes the content place with the same slot settings (layout on the parent stays the same)
    UPanelWidget* Parent = Content->GetParent();
    if (auto SingleChildParent = Cast<UContentWidget>(Parent))
    {
        SingleChildParent->SetContent(Limit);
    }
    else if (IsValid(Parent))
    {
        Parent->InsertChildAt(Parent->GetChildIndex(Content), Limit, Content->Slot);
        Parent->RemoveChild(Content);
    }
    else if (WidgetTree->RootWidget == Content)
    {
        WidgetTree->RootWidget = Limit;
    }
    Limit->SetContent(Scroll);
    // Focus frames are drawn outside the items & clipped by the scroll box, the inset keeps the edge ones whole
    if (auto ContentSlot = Cast<UScrollBoxSlot>(Scroll->AddChild(Content)))
        ContentSlot->SetPadding(FMargin(SCROLLED_CONTENT_INSET));
    return Limit;
}

void MenuLayoutUtils::ChainVerticalNavigation(TConstArrayView<UWidget*> Items, bool bStopAfterLast)
{
    for (int32 i = 0; i < Items.Num(); ++i)
    {
        if (i > 0) Items[i]->SetNavigationRuleExplicit(EUINavigation::Up, Items[i - 1]);
        else Items[i]->SetNavigationRuleBase(EUINavigation::Up, EUINavigationRule::Stop);

        if (i + 1 < Items.Num()) Items[i]->SetNavigationRuleExplicit(EUINavigation::Down, Items[i + 1]);
        else if (bStopAfterLast) Items[i]->SetNavigationRuleBase(EUINavigation::Down, EUINavigationRule::Stop);
    }
}

int32 MenuLayoutUtils::FindActiveButton(TConstArrayView<UButton*> Buttons, bool bPreferHovered)
{
    const int32 Hovered = bPreferHovered
        ? Buttons.IndexOfByPredicate([] (const UButton* Button) { return IsValid(Button) && Button->IsHovered(); })
        : INDEX_NONE;
    if (Hovered != INDEX_NONE) return Hovered;
    return Buttons.IndexOfByPredicate([] (const UButton* Button) { return IsValid(Button) && Button->HasAnyUserFocus(); });
}
