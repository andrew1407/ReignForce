// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ClickableReferenceAction/ClickableReferenceActionWidget.h"
#include "Components/RichTextBlock.h"
#include "Components/Button.h"


UClickableReferenceActionWidget::UClickableReferenceActionWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    HoveredTag = TEXT("Hovered");
}

void UClickableReferenceActionWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    if (IsValid(Reference)) Reference->SetText(ReferenceText);
}

void UClickableReferenceActionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetHoveredReferenceState(false);

    if (IsValid(ClickableAction))
    {
        ClickableAction->OnClicked.AddDynamic(this, &UClickableReferenceActionWidget::OnActionClicked);
        ClickableAction->OnHovered.AddDynamic(this, &UClickableReferenceActionWidget::OnActionHovered);
        ClickableAction->OnUnhovered.AddDynamic(this, &UClickableReferenceActionWidget::OnActionUnhovered);
    }
}

void UClickableReferenceActionWidget::OnActionClicked()
{
    OnActionReference();
}

void UClickableReferenceActionWidget::OnActionHovered()
{
    SetHoveredReferenceState(true);
}

void UClickableReferenceActionWidget::OnActionUnhovered()
{
    SetHoveredReferenceState(false);
}

void UClickableReferenceActionWidget::SetHoveredReferenceState(bool bHovered)
{
    if (!IsValid(Reference)) return;
    if (!bHovered) return Reference->SetText(ReferenceText);
    const FString TextToSet = FString::Printf(TEXT("<%s>%s</>"), *HoveredTag, *ReferenceText.ToString());
    Reference->SetText(FText::FromString(TextToSet));
}
