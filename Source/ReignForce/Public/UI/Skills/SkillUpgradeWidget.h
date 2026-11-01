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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillUpgradeRequested, USkillUpgradeWidget*, SelfWidget);

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

	/** Selected skill is activated again by the gamepad or keyboard (A / Enter) */
	UPROPERTY(BlueprintAssignable)
	FOnSkillUpgradeRequested OnUpgradeRequested;

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
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SkillsProgression", meta = (BindWidget))
	TObjectPtr<UButton> ActionButton;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillsProgression|State", meta = (AllowPrivateAccess = true))
	TMap<EStageProgress, FSkillProgressionStyle> ProgressStates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillsProgression|Sounds", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> OnClickOnActionSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillsProgression|Sounds", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> OnHoverActionSound;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bFocused;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bSelected;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EStageProgress ProgressState;

	UFUNCTION()
	void OnActionClick();

	UFUNCTION()
	void OnActionHover();

	/** Images of the current progress state, kept referenced so they aren't garbage collected after the async load */
	UPROPERTY()
	TArray<TObjectPtr<UTexture2D>> LoadedImages;

	/** Cursor position of the last hover, hover moves the focus only after a real mouse move */
	FVector2D HoverCursorPosition = FVector2D::ZeroVector;

	/** Progress state which images are being loaded (the last requested one wins) */
	EStageProgress LoadingState = EStageProgress::None;

	void SetFocusHighlight(bool bState);
	void SetActionSkillImage(UTexture2D* Image);
	void ApplyStateImage();
};
