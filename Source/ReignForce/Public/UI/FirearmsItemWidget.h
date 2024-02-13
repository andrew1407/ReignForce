// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WeaponItemWidget.h"
#include "FirearmsItemWidget.generated.h"


class UTextBlock;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UFirearmsItemWidget : public UWeaponItemWidget
{
	GENERATED_BODY()

public:
	UFirearmsItemWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void SetWeaponSelected(bool bState) override;
	virtual void SetItemVisible(bool bState) override;

	UFUNCTION(BlueprintCallable)
	void SetWeaponAmmoValue(int32 Value);

	UFUNCTION(BlueprintCallable)
	void SetAmmoStockValue(int32 Value);

	FORCEINLINE UTextBlock* GetWeaponAmmo() const { return WeaponAmmo; }
	FORCEINLINE UTextBlock* GetAmmoStock() const { return AmmoStock; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (BindWidget))
	TObjectPtr<UTextBlock> WeaponAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (BindWidget))
	TObjectPtr<UTextBlock> AmmoStock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (BindWidget))
	TObjectPtr<UTextBlock> Separator;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = true))
	FLinearColor NoAmmoColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = true))
	FLinearColor HasAmmoColor;

	void SetTextValue(int32 Value, UTextBlock* TextBlock);
	void StyleSeparator(int32 Value);
};
