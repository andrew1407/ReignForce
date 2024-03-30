// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClickableReferenceActionWidget.generated.h"


class URichTextBlock;
class UButton;

/**
 * 
 */
UCLASS(Abstract)
class REIGNFORCE_API UClickableReferenceActionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UClickableReferenceActionWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	FORCEINLINE FText GetReferenceText() const { return ReferenceText; }
protected:
	virtual void NativePreConstruct() override;
    virtual void NativeConstruct() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ClickableReferenceAction", meta = (BindWidget))
    TObjectPtr<URichTextBlock> Reference;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ClickableReferenceAction", meta = (BindWidget))
    TObjectPtr<UButton> ClickableAction;

	UFUNCTION(BlueprintNativeEvent, Category = "ClickableReferenceAction")
    void OnActionReference();

    virtual void OnActionReference_Implementation() {}

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reference", meta = (AllowPrivateAccess = true))
	FText ReferenceText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reference", meta = (AllowPrivateAccess = true))
	FString HoveredTag;

	UFUNCTION()
	void OnActionClicked();

	UFUNCTION()
	void OnActionHovered();

	UFUNCTION()
	void OnActionUnhovered();

	void SetHoveredReferenceState(bool bHovered);
};
