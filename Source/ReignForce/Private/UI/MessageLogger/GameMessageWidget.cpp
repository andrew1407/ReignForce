// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MessageLogger/GameMessageWidget.h"
#include "Components/TextBlock.h"


void UGameMessageWidget::SetMessageText(const FText& ToSet)
{
    if (IsValid(Message)) Message->SetText(ToSet);
}
