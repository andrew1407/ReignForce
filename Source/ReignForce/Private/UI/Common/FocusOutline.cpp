// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/FocusOutline.h"
#include "Brushes/SlateColorBrush.h"
#include "Rendering/DrawElements.h"
#include "Components/ScrollBox.h"
#include "Components/Widget.h"


int32 FocusOutline::Paint(const UWidget* Target, const FFocusOutlineStyle& Style, FSlateWindowElementList& OutDrawElements, int32 LayerId)
{
    if (!IsValid(Target) || Style.Thickness <= 0 || Style.Color.A <= 0) return LayerId;

    // Paint space geometry (not the tick one, which is in the desktop space) of the target painted this frame
    const FGeometry& Geometry = Target->GetPaintSpaceGeometry();
    const FVector2f Size = Geometry.GetLocalSize();
    if (Size.X <= 0 || Size.Y <= 0) return LayerId;

    const UScrollBox* ScrollBox = nullptr;
    for (const UWidget* Parent = Target->GetParent(); IsValid(Parent) && !ScrollBox; Parent = Parent->GetParent())
        ScrollBox = Cast<UScrollBox>(Parent);
    if (IsValid(ScrollBox)) OutDrawElements.PushClip(FSlateClippingZone(ScrollBox->GetPaintSpaceGeometry()));

    // Frame of 4 strips around the target (outside its bounds), the brush color comes from the tint
    static const FSlateColorBrush Brush(FLinearColor::White);
    const float T = Style.Thickness;
    const float G = Style.Gap;
    const float Outer = G + T;
    const FBox2f Strips[] = {
        FBox2f(FVector2f(-Outer, -Outer), FVector2f(Size.X + Outer, -G)),                // top
        FBox2f(FVector2f(-Outer, Size.Y + G), FVector2f(Size.X + Outer, Size.Y + Outer)), // bottom
        FBox2f(FVector2f(-Outer, -G), FVector2f(-G, Size.Y + G)),                         // left
        FBox2f(FVector2f(Size.X + G, -G), FVector2f(Size.X + Outer, Size.Y + G)),         // right
    };
    const int32 OutlineLayer = LayerId + 1;
    for (const FBox2f& Strip : Strips)
    {
        const FPaintGeometry PaintGeometry = Geometry.ToPaintGeometry(Strip.GetSize(), FSlateLayoutTransform(Strip.Min));
        FSlateDrawElement::MakeBox(OutDrawElements, OutlineLayer, PaintGeometry, &Brush, ESlateDrawEffect::None, Style.Color);
    }

    if (IsValid(ScrollBox)) OutDrawElements.PopClip();
    return OutlineLayer;
}
