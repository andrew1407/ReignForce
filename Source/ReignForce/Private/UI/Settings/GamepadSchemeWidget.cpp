// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Settings/GamepadSchemeWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/Texture2D.h"
#include "Rendering/DrawElementTypes.h"
#include "Styling/CoreStyle.h"

#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

#include "Settings/ReignForceSettings.h"


namespace
{
    constexpr float LINE_ELBOW_LENGTH = 28;
    constexpr float LINE_LABEL_OFFSET = 10;
    constexpr float ANCHOR_SIZE = 7;

    FGamepadSchemeCallout MakeCallout(const FKey& Key, const TCHAR* Label, FVector2D Anchor, float LabelHeight, bool bLeftSide)
    {
        FGamepadSchemeCallout Callout;
        Callout.Key = Key;
        Callout.Label = FText::FromString(Label);
        Callout.Anchor = Anchor;
        Callout.LabelHeight = LabelHeight;
        Callout.bLeftSide = bLeftSide;
        return Callout;
    }
}

UGamepadSchemeWidget::UGamepadSchemeWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    SchemeSize = FVector2D(1240, 560);
    ImageWidth = 620;
    LabelsGap = 70;
    BackgroundColor = FLinearColor(0, 0, 0, .85f);
    LineColor = FLinearColor(1, 1, 1, .55f);
    AnchorColor = FLinearColor(.75f, .5f, 0.f);
    FocusedButtonScale = 1.05f;
    bBuilt = false;
    ImageSize = FVector2D::ZeroVector;

    // Anchors are measured on the controller image (T_GamepadScheme), actions follow the gameplay mapping context
    constexpr bool bLeft = true;
    Callouts = {
        MakeCallout(EKeys::Gamepad_LeftTrigger, TEXT("Aim"), FVector2D(.25f, .08f), .08f, bLeft),
        MakeCallout(EKeys::Gamepad_LeftShoulder, TEXT("Previous weapon"), FVector2D(.19f, .14f), .19f, bLeft),
        MakeCallout(EKeys::Gamepad_Left2D, TEXT("Move"), FVector2D(.258f, .3f), .31f, bLeft),
        MakeCallout(EKeys::Gamepad_Special_Left, TEXT("Save progress"), FVector2D(.431f, .313f), .43f, bLeft),
        MakeCallout(EKeys::Gamepad_Special_Right, TEXT("Pause menu"), FVector2D(.57f, .313f), .55f, bLeft),
        MakeCallout(EKeys::Gamepad_DPad_Up, TEXT("Up: skills menu\nDown: start round"), FVector2D(.373f, .524f), .69f, bLeft),
        MakeCallout(EKeys::Gamepad_RightTrigger, TEXT("Shoot / attack"), FVector2D(.729f, .082f), .08f, !bLeft),
        MakeCallout(EKeys::Gamepad_RightShoulder, TEXT("Next weapon"), FVector2D(.812f, .136f), .18f, !bLeft),
        MakeCallout(EKeys::Gamepad_FaceButton_Top, TEXT("Movement mode"), FVector2D(.745f, .222f), .29f, !bLeft),
        MakeCallout(EKeys::Gamepad_FaceButton_Right, TEXT("Attack\nBack in menus"), FVector2D(.81f, .315f), .4f, !bLeft),
        MakeCallout(EKeys::Gamepad_FaceButton_Bottom, TEXT("Jump\nSelect in menus"), FVector2D(.745f, .4f), .53f, !bLeft),
        MakeCallout(EKeys::Gamepad_FaceButton_Left, TEXT("Reload"), FVector2D(.68f, .315f), .65f, !bLeft),
        MakeCallout(EKeys::Gamepad_Right2D, TEXT("Camera\nClick: movement mode"), FVector2D(.624f, .513f), .77f, !bLeft),
    };
}

TSharedRef<SWidget> UGamepadSchemeWidget::RebuildWidget()
{
    // Created without a style template, the engine default button style is used
    if (!bBuilt) Setup(nullptr);
    return Super::RebuildWidget();
}

void UGamepadSchemeWidget::Setup(UButton* StyleTemplate)
{
    if (bBuilt || !IsValid(WidgetTree)) return;
    bBuilt = true;

    const FSlateFontInfo TitleFont = MakeFont(StyleTemplate, 44);
    const FSlateFontInfo LabelFont = MakeFont(StyleTemplate, 18);
    const FSlateFontInfo ButtonFont = MakeFont(StyleTemplate, 22);
    const FSlateFontInfo HintFont = MakeFont(StyleTemplate, 14);
    const FSlateColor TextColor = FLinearColor::White;

    UVerticalBox* Column = ConstructMenuColumn(FText::FromString(TEXT("Gamepad")), TitleFont, SchemeSize.X, BackgroundColor);

    auto SchemeFrame = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SchemeFrame"));
    SchemeFrame->SetWidthOverride(SchemeSize.X);
    SchemeFrame->SetHeightOverride(SchemeSize.Y);
    Scheme = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Scheme"));
    SchemeFrame->SetContent(Scheme);
    Column->AddChildToVerticalBox(SchemeFrame);

    UTexture2D* Texture = UReignForceSettings::Get()->GamepadSchemeImage.LoadSynchronous();
    if (IsValid(Texture) && Texture->GetSurfaceWidth() > 0)
    {
        ImageSize = FVector2D(ImageWidth, ImageWidth * Texture->GetSurfaceHeight() / Texture->GetSurfaceWidth());
        auto Image = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Controller"));
        Image->SetBrushFromTexture(Texture);
        if (UCanvasPanelSlot* ImageSlot = Scheme->AddChildToCanvas(Image))
        {
            ImageSlot->SetPosition(GetImageOrigin());
            ImageSlot->SetSize(ImageSize);
        }
    }

    for (const FGamepadSchemeCallout& Callout : Callouts)
    {
        UTextBlock* Label = MakeText(Callout.Label, LabelFont, TextColor);
        Label->SetJustification(Callout.bLeftSide ? ETextJustify::Right : ETextJustify::Left);
        UCanvasPanelSlot* LabelSlot = Scheme->AddChildToCanvas(Label);
        if (!IsValid(LabelSlot)) continue;
        LabelSlot->SetAutoSize(true);
        LabelSlot->SetAlignment(FVector2D(Callout.bLeftSide ? 1 : 0, .5f));
        LabelSlot->SetPosition(GetLabelEdge(Callout));
    }

    BackButton = MakeButton(StyleTemplate, MakeText(FText::FromString(TEXT("Back")), ButtonFont, TextColor), TEXT("BackButton"));
    BackButton->OnClicked.AddDynamic(this, &UReignForceActivatableWidget::RequestClose);
    RegisterFocusableButton(BackButton);
    if (UVerticalBoxSlot* BackSlot = Column->AddChildToVerticalBox(BackButton))
    {
        BackSlot->SetHorizontalAlignment(HAlign_Center);
        BackSlot->SetPadding(FMargin(0, 24, 0, 12));
    }

    UTextBlock* HintText = MakeText(FText::FromString(TEXT("Esc / B - back")), HintFont, FLinearColor(1, 1, 1, .7f));
    if (UVerticalBoxSlot* HintSlot = Column->AddChildToVerticalBox(HintText))
        HintSlot->SetHorizontalAlignment(HAlign_Center);
}

int32 UGamepadSchemeWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    int32 MaxLayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
    if (!IsValid(Scheme) || ImageSize.IsZero()) return MaxLayerId;

    // Leader lines go from the labels (horizontal elbow) to the controls, drawn in this widget space over the scheme
    const FGeometry& SchemeGeometry = Scheme->GetCachedGeometry();
    auto ToLocal = [&] (const FVector2D& SchemePoint)
    {
        return FVector2f(AllottedGeometry.AbsoluteToLocal(SchemeGeometry.LocalToAbsolute(SchemePoint)));
    };
    const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
    const int32 LinesLayer = MaxLayerId + 1;

    for (const FGamepadSchemeCallout& Callout : Callouts)
    {
        const float Direction = Callout.bLeftSide ? 1 : -1;
        const FVector2D Start = GetLabelEdge(Callout) + FVector2D(Direction * LINE_LABEL_OFFSET, 0);
        const FVector2D Elbow = Start + FVector2D(Direction * LINE_ELBOW_LENGTH, 0);
        const FVector2D Anchor = GetImageOrigin() + Callout.Anchor * ImageSize;

        TArray<FVector2f> Points = { ToLocal(Start), ToLocal(Elbow), ToLocal(Anchor) };
        constexpr bool bAntialias = true;
        FSlateDrawElement::MakeLines(OutDrawElements, LinesLayer, AllottedGeometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, LineColor, bAntialias, 1.5f);

        const FVector2f DotSize(ANCHOR_SIZE);
        for (const FVector2f& Dot : { Points[0], Points[2] })
        {
            const FPaintGeometry DotGeometry = AllottedGeometry.ToPaintGeometry(DotSize, FSlateLayoutTransform(Dot - DotSize / 2));
            FSlateDrawElement::MakeBox(OutDrawElements, LinesLayer + 1, DotGeometry, WhiteBrush, ESlateDrawEffect::None, AnchorColor);
        }
    }
    return LinesLayer + 1;
}

FVector2D UGamepadSchemeWidget::GetImageOrigin() const
{
    return (SchemeSize - ImageSize) / 2;
}

FVector2D UGamepadSchemeWidget::GetLabelEdge(const FGamepadSchemeCallout& Callout) const
{
    const float X = Callout.bLeftSide ? GetImageOrigin().X - LabelsGap : GetImageOrigin().X + ImageSize.X + LabelsGap;
    return FVector2D(X, Callout.LabelHeight * SchemeSize.Y);
}
