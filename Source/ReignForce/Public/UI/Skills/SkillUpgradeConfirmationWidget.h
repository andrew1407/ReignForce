// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Common/ReignForceActivatableWidget.h"
#include "Styling/SlateTypes.h"
#include "SkillUpgradeConfirmationWidget.generated.h"


class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillUpgradeConfirmation, bool, bAccepted);

/**
 * Asks whether the skill stage should be upgraded: a dialog over the blurred skills menu (A / Enter - upgrade, B / Esc - cancel)
 */
UCLASS()
class REIGNFORCE_API USkillUpgradeConfirmationWidget : public UReignForceActivatableWidget
{
	GENERATED_BODY()

public:
	USkillUpgradeConfirmationWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(BlueprintAssignable)
	FOnSkillUpgradeConfirmation OnConfirmation;

	/** Builds the dialog in the pause menu modals style taking only the font face from the template (should be called before adding to the viewport) */
	UFUNCTION(BlueprintCallable)
	void Setup(UButton* StyleTemplate, const FText& SkillTitle, const FText& Details);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;
	virtual bool HandleBackAction() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog", meta = (ClampMin = 0, UIMin = 0))
	float BlurStrength;

	/** Full screen tint over the blurred skills menu (same as the pause menu modals background) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog")
	FLinearColor DimColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog")
	FLinearColor FrameColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog")
	FLinearColor PanelColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog|Buttons")
	FLinearColor ButtonColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog|Buttons")
	FLinearColor ButtonHoveredColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog|Buttons")
	FLinearColor ButtonPressedColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog|Buttons")
	FLinearColor ButtonOutlineColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialog", meta = (ClampMin = 0, UIMin = 0))
	float PanelWidth;

private:
	UPROPERTY()
	TObjectPtr<UButton> AcceptButton;

	UPROPERTY()
	TObjectPtr<UButton> CancelButton;

	bool bBuilt;

	FButtonStyle MakeModalButtonStyle() const;
	UButton* MakeModalButton(const FText& Label, const FSlateFontInfo& Font, FName Name);

	UFUNCTION()
	void HandleAccept();

	UFUNCTION()
	void HandleCancel();
};
