// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skills/UpgradesProgressWidget.h"
#include "Engine/AssetManager.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"


UUpgradesProgressWidget::UUpgradesProgressWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	StageProgress = EStageProgress::None;
    ProgressStyle.Color = FLinearColor::White;
    ProgressCompletedStyle.Color = FLinearColor::Yellow;
}

void UUpgradesProgressWidget::LoadProgressionState(bool bCompleted)
{
	EStageProgress ToSet = bCompleted ? EStageProgress::Completed : EStageProgress::InProgress;
    if (StageProgress == ToSet) return;

    StageProgress = ToSet;
	FUpgradesProgressStateStyle* Style = IsProgressionCompleted() ? &ProgressCompletedStyle : &ProgressStyle;
    UTextBlock* TextBlocks[] = { UpgradesCurrent, UpgradesTotal, Splitter };
    for (auto& Text : TextBlocks)
    {
        if (IsValid(Text)) Text->SetColorAndOpacity(Style->Color);
    }

    UAssetManager::GetStreamableManager().RequestAsyncLoad(Style->Image.ToSoftObjectPath(), [this, Style]
    {
        if (Style->Image.IsValid() && IsValid(Icon))
        {
            FSlateBrush Brush;
            Brush.SetResourceObject(Style->Image.Get());
            Icon->SetBrush(Brush);
        }
    });
}

void UUpgradesProgressWidget::SetUpgradesCurrentCount(int32 Value)
{
    if (!IsValid(UpgradesCurrent) || Value < 0) return;
    UpgradesCurrent->SetText(FText::AsNumber(Value));
}

void UUpgradesProgressWidget::SetUpgradesTotalCount(int32 Value)
{
    if (!IsValid(UpgradesTotal) || Value < 0) return;
    UpgradesTotal->SetText(FText::AsNumber(Value));
}

void UUpgradesProgressWidget::ProvideWithState(int32 Current, int32 Total)
{
    SetUpgradesCurrentCount(Current);
    SetUpgradesTotalCount(Total);
    bool bCompleted = Current == Total;
    LoadProgressionState(bCompleted);
}
