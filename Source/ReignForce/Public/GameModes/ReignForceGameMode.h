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
class AShooterHUD;

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

	UFUNCTION(BlueprintCallable)
	void AtivateRoundReloadCoundown(bool bResetIfStarted = false);

	FORCEINLINE UShooterEquipmentDataAsset* GetShooterEquipment() const { return ShooterEquipment; }
	FORCEINLINE UBackgroundMusicComponent* GetBackgroundMusicComponent() const { return BackgroundMusicComponent; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons", meta = (AllowPrivateAccess = true))
	TObjectPtr<UShooterEquipmentDataAsset> ShooterEquipment;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons", meta = (AllowPrivateAccess = true))
	TObjectPtr<UBackgroundMusicComponent> BackgroundMusicComponent;

	UPROPERTY()
	FTimerHandle ReloadRoundTimer;

	UFUNCTION()
	void HandleRoundStart(bool bStartedByPlayer);

	UFUNCTION()
	void HandleRoundEnd(bool bPlayerWin);

	UFUNCTION()
	void EndRoundOnPlayerDeath(AShooterCharacter* Character, AActor* Cause);

	UFUNCTION()
	void OnReloadLevelTimerHandle();

	AShooterHUD* GetShooterHUD() const;
};
