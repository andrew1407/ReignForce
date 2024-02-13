// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skills/SkillDescriptionWidget.h"

#include "Components/Widget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"

#include "UI/Skills/SkillStageInfo.h"


USkillDescriptionWidget::USkillDescriptionWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    bStagesEnumerated = true;
}

void USkillDescriptionWidget::NativeDestruct()
{
    if (IsValid(UpgradeSkillButton)) UpgradeSkillButton->OnClicked.Clear();

    Super::NativeDestruct();
}

void USkillDescriptionWidget::ClearDescription()
{
    if (IsValid(Title)) Title->SetText(FText::GetEmpty());

    if (IsValid(SkillImage))
    {
        FSlateBrush Brush;
        Brush.SetResourceObject(nullptr);
        SkillImage->SetBrush(Brush);
    }

    ClearStagesList();
    SetSkillUpdateToSpend(0);
}

void USkillDescriptionWidget::SetTitle(const FName& SkillName, int32 StagesTotal, int32 CurrentStage)
{
    if (!IsValid(Title)) return;
    const FString ToSet = FString::Printf(TEXT("%s\n(%d/%d)"), *SkillName.ToString(), CurrentStage, StagesTotal);
    Title->SetText(FText::FromString(ToSet));
}

void USkillDescriptionWidget::SetSkillImage(UTexture2D* Image)
{
    if (!IsValid(SkillImage)) return;

    FSlateBrush Brush;
    Brush.SetResourceObject(Image);
    SkillImage->SetBrush(Brush);
}

void USkillDescriptionWidget::ClearStagesList()
{
    if (!IsValid(StagesBox)) return;

    for (UWidget* Widget : StagesBox->GetAllChildren())
    {
        if (Widget->IsA(USkillStageInfo::StaticClass())) Widget->RemoveFromParent();
    }
}

void USkillDescriptionWidget::FillStagesList(const TArray<FSkillStageDescriptionDetails>& Info, bool bClearPreviously)
{
    if (!IsValid(StagesBox) || Info.IsEmpty() || !IsValid(StageInfoClass)) return;

    if (bClearPreviously) ClearStagesList();

    for (int n = 0, i = 0; i < Info.Num(); ++i)
    {
        auto StageInfo = CreateWidget<USkillStageInfo>(this, StageInfoClass);
        if (!IsValid(StageInfo)) continue;
        const FSkillStageDescriptionDetails& Details = Info[i];
        const int32 OrderNumber = bStagesEnumerated ? ++n : -1;
        StageInfo->SetDescription(Details.Entries, Details.StageProgress, OrderNumber);
        StagesBox->AddChild(StageInfo);
    }
}

void USkillDescriptionWidget::SetSkillUpdateToSpend(int32 ToSpend)
{
    if (!IsValid(UpgradeSkillButtonDescription)) return;
    const FString ToSet = FString::Printf(TEXT("Upgrade (%d)"), ToSpend);
    UpgradeSkillButtonDescription->SetText(FText::FromString(ToSet));
}

void USkillDescriptionWidget::SetUpgradeButtonVisibility(bool bState)
{
    if (!IsValid(UpgradeSkillButton)) return;

    const ESlateVisibility Buttonisibility = bState ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
    UpgradeSkillButton->SetVisibility(Buttonisibility);
}

void USkillDescriptionWidget::SetUpgradeButtonEnabled(bool bState)
{
    if (!IsValid(UpgradeSkillButton)) return;
    UpgradeSkillButton->SetIsEnabled(bState);
}

bool USkillDescriptionWidget::GetUpgradeButtonEnabled() const
{
    return IsValid(UpgradeSkillButton) && UpgradeSkillButton->GetIsEnabled();
}
