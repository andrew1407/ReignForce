// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/WeaponType.h"
#include "WeaponSlotsSystem.generated.h"


class AWeapon;
class UAnimMontage;
class USkeletalMeshComponent;
class UAnimInstance;

USTRUCT(BlueprintType)
struct FSlotDetails
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UAnimMontage> EquipAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UAnimMontage> UnequipAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SocketBinding;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REIGNFORCE_API UWeaponSlotsSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponSlotsSystem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static bool IsFirearmsType(EWeaponType WeaponType);

	#pragma region SLOT_MODIFIERS

	UFUNCTION(BlueprintCallable)
	bool CreateAttachedWeapon(TSubclassOf<AWeapon> WeaponClass, EWeaponType Slot);

	UFUNCTION(BlueprintCallable)
	bool AddWeapon(AWeapon* Weapon, EWeaponType Slot);

	UFUNCTION(BlueprintCallable)
	bool RemoveWeapon(EWeaponType Slot);

	UFUNCTION(BlueprintCallable)
	void RemoveAllWeapons();

	#pragma endregion

	#pragma region STATE_GETTERS

	UFUNCTION(BlueprintPure)
	bool IsActiveWeaponValid() const;

	UFUNCTION(BlueprintPure)
	bool IsWeaponInSlotValid(EWeaponType Slot) const;

	UFUNCTION(BlueprintPure)
	bool HasFreeSlot(EWeaponType Slot) const;

	UFUNCTION(BlueprintPure)
	AWeapon* GetWeaponFromSlot(EWeaponType Slot) const;

	UFUNCTION(BlueprintPure)
	AWeapon* GetActiveWeapon() const;

	UFUNCTION(BlueprintPure)
	bool IsFirearmsActive() const;

	UFUNCTION(BlueprintPure)
	TArray<EWeaponType> GetAvailableWeaponSlots() const;

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsUnarmedActive() const { return ActiveWeapon == EWeaponType::Unarmed; }

	#pragma endregion

	#pragma region SWAP_ANIMATION_HANDLERS

	UFUNCTION(BlueprintCallable)
	bool EquipFromSlot(EWeaponType Slot, float Rate = 1);

	UFUNCTION(BlueprintCallable)
	bool HideActiveWeapon(float Rate = 1);

	UFUNCTION(BlueprintCallable)
	bool SwapActiveWeapon(EWeaponType Slot, float Rate = 1);

	#pragma endregion

	#pragma region WEAPON_ATTACH_CONTROL

	UFUNCTION(BlueprintCallable)
	bool SetReservedToActive();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void ClearReservedWeaponSlot() { ReservedSlot = EWeaponType::None; }

	UFUNCTION(BlueprintCallable)
	bool AttachWeaponToSlot(EWeaponType Slot);

	UFUNCTION(BlueprintCallable)
	bool AttachToActive(EWeaponType Slot);

	UFUNCTION(BlueprintCallable)
	bool AttachToActiveDirectly(EWeaponType Slot);

	UFUNCTION(BlueprintCallable)
	bool AttachActiveToSlotDirectly(EWeaponType Slot, EWeaponType Default = EWeaponType::Unarmed);

	#pragma endregion

	FORCEINLINE EWeaponType GetActiveWeaponSlot() const { return ActiveWeapon; }
	FORCEINLINE EWeaponType GetReservedWeaponSlot() const { return ReservedSlot; }

private:
	UPROPERTY(BlueprintReadWrite, Category = "Slots", meta = (AllowPrivateAccess = true))
	EWeaponType ActiveWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slots", meta = (AllowPrivateAccess = true))
	FName ActiveWeaponSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slots", meta = (AllowPrivateAccess = true))
	TMap<EWeaponType, FSlotDetails> SlotsDescription;

	UPROPERTY(BlueprintReadWrite, Category = "Slots", meta = (AllowPrivateAccess = true))
	TMap<EWeaponType, AWeapon*> Weapons;

	EWeaponType ReservedSlot;

	USkeletalMeshComponent* GetSkeletalMesh();
	UAnimInstance* GetMeshAnimInstance();
	bool TryPlayMontage(UAnimInstance* AnimInstance, UAnimMontage* AnimMontage, float Rate);
};
