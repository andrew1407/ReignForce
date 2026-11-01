// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Modals/ConfirmationModalWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"


UConfirmationModalWidget::UConfirmationModalWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    ModalFocusMode = EModalFocusMode::None;
    bIsModal = true;
}

void UConfirmationModalWidget::NativeConstruct()
{
    // Preferred button is registered first to get the initial (gamepad & keyboard) focus, cancel is the safe default
    bool bAcceptFirst = ModalFocusMode == EModalFocusMode::FocusOnAccept;
    RegisterFocusableButton(bAcceptFirst ? AcceptButton : CancelButton);
    RegisterFocusableButton(bAcceptFirst ? CancelButton : AcceptButton);
    SetBackActionButton(CancelButton);

    Super::NativeConstruct();

    if (IsValid(AcceptButton))
    {
        AcceptButton->OnClicked.AddDynamic(this, &UConfirmationModalWidget::OnAccept);
        if (ModalFocusMode == EModalFocusMode::FocusOnAccept) FocusButton(AcceptButton);
    }

    if (IsValid(CancelButton))
    {
        CancelButton->OnClicked.AddDynamic(this, &UConfirmationModalWidget::OnCancel);
        if (ModalFocusMode == EModalFocusMode::FocusOnCancel) FocusButton(CancelButton);
    }
}

bool UConfirmationModalWidget::HandleBackAction()
{
    OnCancel();
    return true;
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