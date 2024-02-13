// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/FirearmsItemWidget.h"
#include "Components/TextBlock.h"


UFirearmsItemWidget::UFirearmsItemWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NoAmmoColor = FLinearColor::Red;
	HasAmmoColor = FLinearColor::White;
}

void UFirearmsItemWidget::SetWeaponSelected(bool bState)
{
	Super::SetWeaponSelected(bState);

	for (auto& TextBlock : { WeaponAmmo, AmmoStock, Separator })
	{
		if (!IsValid(TextBlock)) return;
		FLinearColor TextColor = TextBlock->GetColorAndOpacity().GetSpecifiedColor();
		TextColor.A = bState ? 1 : InactiveOpacity;
		TextBlock->SetColorAndOpacity(TextColor);
	}
}

void UFirearmsItemWidget::SetItemVisible(bool bState)
{
	Super::SetItemVisible(bState);

	const ESlateVisibility TextVisibility = bState ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	for (auto& TextBlock : { WeaponAmmo, AmmoStock, Separator })
	{
		if (IsValid(TextBlock)) TextBlock->SetVisibility(TextVisibility);
	}
}

void UFirearmsItemWidget::SetWeaponAmmoValue(int32 Value)
{
	SetTextValue(Value, WeaponAmmo);
	StyleSeparator(Value);
}

void UFirearmsItemWidget::SetAmmoStockValue(int32 Value)
{
	SetTextValue(Value, AmmoStock);
	StyleSeparator(Value);
}

void UFirearmsItemWidget::SetTextValue(int32 Value, UTextBlock* TextBlock)
{
	if (!IsValid(TextBlock)) return;
	FLinearColor TextColor = Value == 0 ? NoAmmoColor : HasAmmoColor;
	TextColor.A = WeaponAmmo->GetColorAndOpacity().GetSpecifiedColor().A;
	TextBlock->SetColorAndOpacity(TextColor);
	TextBlock->SetText(FText::AsNumber(Value));
}

void UFirearmsItemWidget::StyleSeparator(int32 Value)
{
	if (!(IsValid(WeaponAmmo) && IsValid(AmmoStock) && IsValid(Separator))) return;
	const FText Zero = FText::FromString(TEXT("0"));
	bool bRed = WeaponAmmo->GetText().EqualTo(Zero) && AmmoStock->GetText().EqualTo(Zero);
	FLinearColor TextColor = bRed ? NoAmmoColor : HasAmmoColor;
	TextColor.A = Separator->GetColorAndOpacity().GetSpecifiedColor().A;
	Separator->SetColorAndOpacity(TextColor);
}
