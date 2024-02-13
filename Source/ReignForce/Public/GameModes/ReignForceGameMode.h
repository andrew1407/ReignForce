// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Weapons/WeaponType.h"
#include "ReignForceGameMode.generated.h"


class UShooterEquipmentDataAsset;
class UBackgroundMusicComponent;
class AWeapon;
class AShooterCharacter;

UCLASS(minimalapi)
class AReignForceGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AReignForceGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure)
	bool IsShooterEquipmentValid() const;

	UFUNCTION(BlueprintPure)
	TSoftClassPtr<AWeapon> GetWeaponModel(EWeaponType WeaponType, int32 Index) const;

	UFUNCTION(BlueprintCallable)
	void RefillShooterAmmo(AShooterCharacter* Shooter);

	UFUNCTION(BlueprintCallable)
	void RefillPlayerAmmo();

	FORCEINLINE UShooterEquipmentDataAsset* GetShooterEquipment() const { return ShooterEquipment; }
	FORCEINLINE UBackgroundMusicComponent* GetBackgroundMusicComponent() const { return BackgroundMusicComponent; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons", meta = (AllowPrivateAccess = true))
	TObjectPtr<UShooterEquipmentDataAsset> ShooterEquipment;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons", meta = (AllowPrivateAccess = true))
	TObjectPtr<UBackgroundMusicComponent> BackgroundMusicComponent;

	UFUNCTION()
	void PlayBackgroundGameplayMusic(bool bStartedByPlayer);

	UFUNCTION()
	void RefillPlayerAmmoOnRoundStart(bool bStartedByPlayer);

	UFUNCTION()
	void StopBackgroundGameplayMusic(bool bPlayerWin);

	UFUNCTION()
	void EndRoundOnPlayerDeath(AShooterCharacter* Character, AActor* Cause);
};
