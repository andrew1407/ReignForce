// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skills/SkillStageInfo.h"
#include "Components/TextBlock.h"


USkillStageInfo::USkillStageInfo(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    ProgressStatusColorDefault = FLinearColor::White;
    ProgressStatusColors =
    {
        { EStageProgress::Locked, FLinearColor::Gray },
        { EStageProgress::InProgress, FLinearColor::White },
        { EStageProgress::Completed, FLinearColor::Green },
    };
}

FText USkillStageInfo::GetDescription() const
{
    return IsValid(StageDescription) ? StageDescription->GetText() : FText();
}

void USkillStageInfo::SetDescription(const FText& Info, EStageProgress StageProgress, int32 Order)
{
    if (!IsValid(StageDescription)) return;

    if (Order > 0)
    {
        const FFormatOrderedArguments Args = { FText::AsNumber(Order), Info };
        const FText ToSet = FText::Format(FText::FromString("{0}. {1}"), Args);
        StageDescription->SetText(ToSet);
    }
    else
    {
        StageDescription->SetText(Info);
    }

    const FLinearColor* ColorPtr = ProgressStatusColors.Find(StageProgress);
    const FLinearColor Color = ColorPtr ? *ColorPtr : ProgressStatusColorDefault;
    StageDescription->SetColorAndOpacity(Color);
}
