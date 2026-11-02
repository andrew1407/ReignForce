// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"
#include "Framework/Text/TextLayout.h"
#include "Templates/SubclassOf.h"


class UButton;
class UTextBlock;
class UVerticalBox;
class USizeBox;
class UWidget;
class UWidgetTree;

/**
 * Natively built menus layout: widgets in the menus look (fonts, buttons, columns) and menu panels rearrangement
 */
namespace MenuLayoutUtils
{
    /** Space around the scrolled menu content (focus frames stay inside the scroll box clipping) */
    constexpr float SCROLLED_CONTENT_INSET = 8;

    /** Title size of the full screen menus (settings, key bindings) */
    constexpr int32 MENU_TITLE_FONT_SIZE = 44;

    #pragma region WIDGETS

    /** Template button label font of the given size (engine default bold font without a template) */
    REIGNFORCE_API FSlateFontInfo MakeFont(const UButton* StyleTemplate, int32 Size);

    /** Shadowed text of the menus (common text block by default) */
    REIGNFORCE_API UTextBlock* MakeText(UWidgetTree* WidgetTree, const FText& Text, const FSlateFontInfo& Font, const FSlateColor& Color, TSubclassOf<UTextBlock> TextClass = nullptr);

    /** Button with the template style (brushes, sounds, colors) holding the content */
    REIGNFORCE_API UButton* MakeButton(UWidgetTree* WidgetTree, UButton* StyleTemplate, UWidget* Content, FName Name);

    /** Clones the template button style & label into a new button placed next to the template (same panel) */
    REIGNFORCE_API UButton* CreateButtonLike(UWidgetTree* WidgetTree, UButton* Template, const FText& Label, FName Name, int32 IndexOffset = 0);

    /** Enables the button or shows it disabled with a see-through, paler orange of its own fill (without a disabled brush Slate only fades the button) */
    REIGNFORCE_API void SetButtonAvailable(UButton* Button, bool bAvailable);

    /** Dimmed fullscreen background (the tree root) with a centered titled column (menu content is added to it) */
    REIGNFORCE_API UVerticalBox* ConstructMenuColumn(UWidgetTree* WidgetTree, const FText& Title, const FSlateFontInfo& TitleFont, float Width, const FLinearColor& Background);

    REIGNFORCE_API FSlateBrush MakeRoundedBrush(const FLinearColor& Fill, const FLinearColor& Outline, float Radius, float OutlineWidth);

    /** The button text: its content, or the label next to the gamepad prompt once the button is registered as focusable */
    REIGNFORCE_API UTextBlock* FindButtonLabel(const UButton* Button);

    /** Text alignment of a text block (the property has no getter, it is read by reflection) */
    REIGNFORCE_API ETextJustify::Type GetJustification(const UTextBlock* Text);

    #pragma endregion

    #pragma region PANELS

    /** Places the items of the panel the given item is in one after another with the given gap (spacers or paddings) */
    REIGNFORCE_API void SetMenuItemsGap(UWidget* Item, float Gap);

    /** Puts the content into a vertical scroll box which takes the content place (slot) inside a size box (returned) */
    REIGNFORCE_API USizeBox* WrapInScrollBox(UWidgetTree* WidgetTree, UWidget* Content);

    /** Up & down go item by item in the given order: nothing above the first one, nothing below the last one when it's stopped */
    REIGNFORCE_API void ChainVerticalNavigation(TConstArrayView<UWidget*> Items, bool bStopAfterLast);

    /** Index of the button a click is for: mouse clicks target the hovered one (when preferred), keyboard & gamepad - the focused one */
    REIGNFORCE_API int32 FindActiveButton(TConstArrayView<UButton*> Buttons, bool bPreferHovered);

    #pragma endregion
}
