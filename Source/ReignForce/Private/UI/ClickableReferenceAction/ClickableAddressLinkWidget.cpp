// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ClickableReferenceAction/ClickableAddressLinkWidget.h"
#include "Kismet/KismetSystemLibrary.h"


void UClickableAddressLinkWidget::OnActionReference_Implementation()
{
    bool bCanLaunchURL = UKismetSystemLibrary::CanLaunchURL(AddressLink);
    if (bCanLaunchURL) return UKismetSystemLibrary::LaunchURL(AddressLink);

    FString Url = GetReferenceText().ToString();
    bCanLaunchURL = UKismetSystemLibrary::CanLaunchURL(Url);
    if (bCanLaunchURL) UKismetSystemLibrary::LaunchURL(Url);
}
