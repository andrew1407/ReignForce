// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Components/WeaponSlotsSystem.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

#include "Weapons/Types/Weapon.h"


// Sets default values for this component's properties
UWeaponSlotsSystem::UWeaponSlotsSystem(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ActiveWeapon = EWeaponType::Unarmed;
	ReservedSlot = EWeaponType::None;
}

bool UWeaponSlotsSystem::IsFirearmsType(EWeaponType WeaponType)
{
	return WeaponType == EWeaponType::Pistol || WeaponType == EWeaponType::Rifle;
}

bool UWeaponSlotsSystem::CreateAttachedWeapon(TSubclassOf<AWeapon> WeaponClass, EWeaponType Slot)
{
	USkeletalMeshComponent* Mesh = GetSkeletalMesh();
	if (!IsValid(Mesh)) return false;
	AWeapon** WeaponPtr = Weapons.Find(Slot);
	if (WeaponPtr && IsValid(*WeaponPtr)) return false;
	FSlotDetails* SlotDetailsPtr = SlotsDescription.Find(Slot);
	if (!SlotDetailsPtr) return false;
	auto Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass);
	if (!IsValid(Weapon)) return false;
	Weapon->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SlotDetailsPtr->SocketBinding);
	return AddWeapon(Weapon, Slot);
}

bool UWeaponSlotsSystem::AddWeapon(AWeapon* Weapon, EWeaponType Slot)
{
	AWeapon** WeaponPtr = Weapons.Find(Slot);
	if (WeaponPtr && IsValid(*WeaponPtr)) return false;
	Weapons.FindOrAdd(Slot, Weapon);
	return true;
}

bool UWeaponSlotsSystem::RemoveWeapon(EWeaponType Slot)
{
	AWeapon** WeaponPtr = Weapons.Find(Slot);
	if (!(WeaponPtr && IsValid(*WeaponPtr))) return false;
	(*WeaponPtr)->Destroy();
	*WeaponPtr = nullptr;
	return Weapons.Remove(Slot) > 0;
}

void UWeaponSlotsSystem::RemoveAllWeapons()
{
	for (const auto &[Slot, _] : Weapons) RemoveWeapon(Slot);
}

bool UWeaponSlotsSystem::IsActiveWeaponValid() const
{
	return IsWeaponInSlotValid(ActiveWeapon);
}

bool UWeaponSlotsSystem::IsWeaponInSlotValid(EWeaponType Slot) const
{
	return Slot == EWeaponType::Unarmed || !HasFreeSlot(Slot) && IsValid(Weapons[Slot]);
}

bool UWeaponSlotsSystem::HasFreeSlot(EWeaponType Slot) const
{
	AWeapon* const* WeaponPtr = Weapons.Find(Slot);
	return !(WeaponPtr && *WeaponPtr);
}

AWeapon* UWeaponSlotsSystem::GetWeaponFromSlot(EWeaponType Slot) const
{
	AWeapon* const* WeaponPtr = Weapons.Find(Slot);
	return WeaponPtr ? *WeaponPtr : nullptr;
}

AWeapon* UWeaponSlotsSystem::GetActiveWeapon() const
{
	AWeapon* const* WeaponPtr = Weapons.Find(ActiveWeapon);
	return WeaponPtr ? *WeaponPtr : nullptr;
}

bool UWeaponSlotsSystem::IsFirearmsActive() const
{
	return IsFirearmsType(ActiveWeapon);
}

TArray<EWeaponType> UWeaponSlotsSystem::GetAvailableWeaponSlots() const
{
	TArray<EWeaponType> Available;
	for (const auto &[Key, Value] : Weapons)
	{
		if (IsValid(Value)) Available.Add(Key);
	}
	return MoveTemp(Available);
}

bool UWeaponSlotsSystem::EquipFromSlot(EWeaponType Slot, float Rate)
{
	if (!IsWeaponInSlotValid(Slot)) return false;
	FSlotDetails* SlotDetailsPtr = SlotsDescription.Find(Slot);
	if (!(SlotDetailsPtr && Weapons.Contains(Slot))) return false;
	if (Slot == EWeaponType::Unarmed || Slot == ActiveWeapon) return false;
	TObjectPtr<UAnimMontage>& EquipMontage = SlotDetailsPtr->EquipAnimation;
	if (!IsValid(EquipMontage)) return false;
	UAnimInstance* AnimInstance = GetMeshAnimInstance();
	if (!IsValid(AnimInstance)) return false;
	ReservedSlot = Slot;
	return TryPlayMontage(AnimInstance, EquipMontage, Rate);
}

bool UWeaponSlotsSystem::HideActiveWeapon(float Rate)
{
	FSlotDetails* SlotDetailsPtr = SlotsDescription.Find(ActiveWeapon);
	if (!SlotDetailsPtr || ActiveWeapon == EWeaponType::Unarmed) return false;
	UAnimInstance* AnimInstance = GetMeshAnimInstance();
	if (!IsValid(AnimInstance)) return false;
	TObjectPtr<UAnimMontage>& UnequipAnimation = SlotDetailsPtr->UnequipAnimation;
	if (!IsValid(UnequipAnimation)) return false;
	return TryPlayMontage(AnimInstance, UnequipAnimation, -Rate);
}

bool UWeaponSlotsSystem::SwapActiveWeapon(EWeaponType Slot, float Rate)
{
	if (!IsWeaponInSlotValid(Slot) || ActiveWeapon == Slot) return false;
	if (ActiveWeapon == EWeaponType::Unarmed) return EquipFromSlot(Slot, Rate);
	ReservedSlot = Slot;
	return HideActiveWeapon(Rate);
}

bool UWeaponSlotsSystem::SetReservedToActive()
{
	if (ReservedSlot == EWeaponType::None) return false;
	ActiveWeapon = ReservedSlot;
	ReservedSlot = EWeaponType::None;
	return true;
}

bool UWeaponSlotsSystem::AttachWeaponToSlot(EWeaponType Slot)
{
	AWeapon** WeaponPtr = Weapons.Find(Slot);
	if (!(WeaponPtr && IsValid(*WeaponPtr))) return false;
	FSlotDetails* SlotDetailsPtr = SlotsDescription.Find(Slot);
	if (!SlotDetailsPtr) return false;
	USkeletalMeshComponent* Mesh = GetSkeletalMesh();
	if (!IsValid(Mesh)) return false;

	bool IsSuccess = (*WeaponPtr)->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SlotDetailsPtr->SocketBinding);
	return IsSuccess;
}

bool UWeaponSlotsSystem::AttachToActive(EWeaponType Slot)
{
	if (Slot == EWeaponType::Unarmed) return false;
	AWeapon** WeaponPtr = Weapons.Find(Slot);
	if (!(WeaponPtr && IsValid(*WeaponPtr))) return false;
	USkeletalMeshComponent* Mesh = GetSkeletalMesh();
	if (!IsValid(Mesh)) return false;

	bool IsSuccess = (*WeaponPtr)->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetIncludingScale, ActiveWeaponSocket);
	return IsSuccess;
}

USkeletalMeshComponent* UWeaponSlotsSystem::GetSkeletalMesh()
{
	auto Character = GetOwner<ACharacter>();
	return IsValid(Character) ? Character->GetMesh() : nullptr;
}

bool UWeaponSlotsSystem::AttachToActiveDirectly(EWeaponType Slot)
{
	bool IsAttached = AttachToActive(Slot);
	if (IsAttached) ActiveWeapon = Slot;
	return IsAttached;
}

bool UWeaponSlotsSystem::AttachActiveToSlotDirectly(EWeaponType Slot, EWeaponType Default)
{
	if (Slot == Default) return false;
	bool IsAttached = AttachWeaponToSlot(Slot);
	if (IsAttached) ActiveWeapon = Default;
	return IsAttached;
}

UAnimInstance* UWeaponSlotsSystem::GetMeshAnimInstance()
{
	USkeletalMeshComponent* SkeletalMesh = GetSkeletalMesh();
	return IsValid(SkeletalMesh) ? SkeletalMesh->GetAnimInstance() : nullptr;
}

bool UWeaponSlotsSystem::TryPlayMontage(UAnimInstance* AnimInstance, UAnimMontage* AnimMontage, float Rate)
{
	float MontageLength = AnimInstance->Montage_Play(AnimMontage, Rate);
	bool IsFailed = MontageLength == 0;
	if (IsFailed) ClearReservedWeaponSlot();
	return !IsFailed;
}
