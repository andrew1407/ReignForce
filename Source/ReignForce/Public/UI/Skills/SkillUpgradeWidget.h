// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Skills/StageProgress.h"
#include "UI/Skills/SkillProgressionStyle.h"
#include "SkillUpgradeWidget.generated.h"


class UButton;
class UImage;

class USkillUpgradeWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnSkillProgressStateChanged, USkillUpgradeWidget*, SelfWidget, EStageProgress, StageProgress, bool, bSelected, EStageProgress, PreviousState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillSelected, USkillUpgradeWidget*, SelfWidget, bool, bState);

/**
 * 
 */
UCLASS()
class REIGNFORCE_API USkillUpgradeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USkillUpgradeWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillsProgression|State")
	FName SkillName;

	UPROPERTY(BlueprintAssignable)
	FOnSkillProgressStateChanged OnSkillProgressStateChanged;

	UPROPERTY(BlueprintAssignable)
	FOnSkillSelected OnSkillSelected;

	UFUNCTION(BlueprintCallable)
	void SetIsSelected(bool bState);

	UFUNCTION(BlueprintCallable)
	void LoadProgressState(EStageProgress State, bool bSelectedState = true);

	UFUNCTION(BlueprintPure)
	UTexture2D* GetBaseSkillIcon() const;

	FORCEINLINE EStageProgress GetProgressState() const { return ProgressState; }
	FORCEINLINE bool IsSelected() const { return bSelected; }
	FORCEINLINE UButton* GetActionButton() const { return ActionButton; }
	FORCEINLINE TPair<EStageProgress, bool> GetCurrentState() const { return { ProgressState, bSelected }; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SkillsProgression", meta = (BindWidget))
	TObjectPtr<UButton> ActionButton;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillsProgression|State", meta = (AllowPrivateAccess = true))
	TMap<EStageProgress, FSkillProgressionStyle> ProgressStates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillsProgression|Sounds", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> OnClickOnActionSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillsProgression|Sounds", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> OnHoverActionSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillsProgression|Focus", meta = (AllowPrivateAccess = true, ClampMin = 1, UIMin = 1))
	float FocusedScale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillsProgression|Focus", meta = (AllowPrivateAccess = true))
	FLinearColor FocusedTint;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bFocused;

	FLinearColor DefaultActionBackground = FLinearColor::White;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bSelected;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EStageProgress ProgressState;

	UFUNCTION()
	void OnActionClick();

	UFUNCTION()
	void OnActionHover();

	void SetFocusHighlight(bool bState);
	void SetActionSkillImage(UTexture2D* Image);
};
