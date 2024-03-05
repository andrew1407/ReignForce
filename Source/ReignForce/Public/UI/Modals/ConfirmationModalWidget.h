// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Modals/ModalFocusMode.h"
#include "ConfirmationModalWidget.generated.h"


class UTextBlock;
class UButton;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnModalWindowWidgetActionResponse, bool, bAccepted);

USTRUCT(BlueprintType)
struct FModalWindowWidgetActionOptions
{
    GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TitleMessage;

	UPROPERTY(BlueprintReadWrite)
	FOnModalWindowWidgetActionResponse OnActionResponse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText AcceptMessage = FText::FromString(TEXT("OK"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText CancelMessage = FText::FromString(TEXT("Cancel"));
};

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UConfirmationModalWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UConfirmationModalWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable)
	void SetModalOptions(const FModalWindowWidgetActionOptions& Options);

	UFUNCTION(BlueprintCallable)
	void SetOnActionResponse(const FOnModalWindowWidgetActionResponse& OnModalWindowWidgetActionResponse);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modal", meta = (BindWidget))
	TObjectPtr<UTextBlock> Title;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modal", meta = (BindWidget))
	TObjectPtr<UButton> AcceptButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modal", meta = (BindWidget))
	TObjectPtr<UTextBlock> AcceptButtonTitle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modal", meta = (BindWidget))
	TObjectPtr<UButton> CancelButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modal", meta = (BindWidget))
	TObjectPtr<UTextBlock> CancelButtonTitle;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modal", meta = (AllowPrivateAccess = true))
	EModalFocusMode ModalFocusMode;

	UPROPERTY()
	FOnModalWindowWidgetActionResponse OnActionResponse;

	UFUNCTION()
	void OnAccept();

	UFUNCTION()
	void OnCancel();
};
