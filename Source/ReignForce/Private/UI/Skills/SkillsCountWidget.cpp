// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skills/SkillsCountWidget.h"
#include "Components/TextBlock.h"


void USkillsCountWidget::SetSkillsCount(int32 Skills)
{
    if (IsValid(Title)) Title->SetText(FText::AsNumber(Skills));
}
