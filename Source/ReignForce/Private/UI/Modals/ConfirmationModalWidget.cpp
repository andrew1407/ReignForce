// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Modals/ConfirmationModalWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"


UConfirmationModalWidget::UConfirmationModalWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    ModalFocusMode = EModalFocusMode::None;
}

void UConfirmationModalWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (IsValid(AcceptButton))
    {
        AcceptButton->OnClicked.AddDynamic(this, &UConfirmationModalWidget::OnAccept);
        if (ModalFocusMode == EModalFocusMode::FocusOnAccept) AcceptButton->SetFocus();
    }
    
    if (IsValid(CancelButton))
    {
        CancelButton->OnClicked.AddDynamic(this, &UConfirmationModalWidget::OnCancel);
        if (ModalFocusMode == EModalFocusMode::FocusOnCancel) CancelButton->SetFocus();
    }
}

void UConfirmationModalWidget::SetModalOptions(const FModalWindowWidgetActionOptions& Options)
{
    OnActionResponse = Options.OnActionResponse;
    if (IsValid(Title)) Title->SetText(Options.TitleMessage);
    if (IsValid(AcceptButtonTitle)) AcceptButtonTitle->SetText(Options.AcceptMessage);
    if (IsValid(CancelButtonTitle)) CancelButtonTitle->SetText(Options.CancelMessage);
}

void UConfirmationModalWidget::SetOnActionResponse(const FOnModalWindowWidgetActionResponse& OnModalWindowWidgetActionResponse)
{
    OnActionResponse = OnModalWindowWidgetActionResponse;
}

void UConfirmationModalWidget::OnAccept()
{
    constexpr bool bAccepcted = true;
    OnActionResponse.ExecuteIfBound(bAccepcted);
}

void UConfirmationModalWidget::OnCancel()
{
    constexpr bool bAccepcted = false;
    OnActionResponse.ExecuteIfBound(bAccepcted);
}

