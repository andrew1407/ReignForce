// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Rounds/RoundProgressWidget.h"
#include "Components/TextBlock.h"


void URoundProgressWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetEnemiesToKillTotalCount(0);
    SetEnemiesToKillCurrentCount(0);
}

void URoundProgressWidget::SetEnemiesToKillTotalCount(int32 Count)
{
    if (IsValid(EnemiesToKillTotal)) EnemiesToKillTotal->SetText(FText::AsNumber(Count));
}

void URoundProgressWidget::SetEnemiesToKillCurrentCount(int32 Count)
{
    if (IsValid(EnemiesToKillCurrent)) EnemiesToKillCurrent->SetText(FText::AsNumber(Count));
}
