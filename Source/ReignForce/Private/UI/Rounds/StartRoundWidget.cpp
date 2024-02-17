// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Rounds/StartRoundWidget.h"
#include "Components/TextBlock.h"


UStartRoundWidget::UStartRoundWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    StartActionKey = TEXT("T");
}

void UStartRoundWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UpdateRoundTextBlockDescrition();
}

FText UStartRoundWidget::GetDescriptionMessage() const
{
    return FText::FromString(FString::Printf(TEXT("Press %s to start a new round."), *StartActionKey.ToString()));
}

void UStartRoundWidget::UpdateRoundTextBlockDescrition()
{
    if (IsValid(RoundStartMessage)) RoundStartMessage->SetText(GetDescriptionMessage());
}
