// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameStates/SkillBalanceAction.h"
#include "GameStates/Components/PlayerProgressionBalance.h"
#include "UpgradesProgressStateComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnSkillsAmountChanged, ESkillBalanceAction, SkillBalanceAction, int32, Current, int32, Updated, int32, Total, int32, Previous);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnUpgradesUnlocked, int32, Current, int32, Updated, int32, Total, int32, Previous);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REIGNFORCE_API UUpgradesProgressStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UUpgradesProgressStateComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	#pragma region EVENTS

	UPROPERTY(BlueprintAssignable)
	FOnSkillsAmountChanged OnSkillsAmountChanged;

	UPROPERTY(BlueprintAssignable)
	FOnUpgradesUnlocked OnUpgradesUnlocked;

	#pragma endregion

	#pragma region SKILLS

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool CanTake(int32 ToTake = 1) const { return Progression.SkillsAvailable >= ToTake && ToTake > 0; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsFullOfSkills() const { return Progression.GetSkillsGained() == MaxSkillsAllowed; }

	UFUNCTION(BlueprintCallable)
	bool SetSkillsCount(int32 ToSet, bool bNotify = true);

	UFUNCTION(BlueprintCallable)
	bool SetSkillsSpentCount(int32 ToSet);

	UFUNCTION(BlueprintCallable)
	bool DropSkills(bool bNotify = true);

	UFUNCTION(BlueprintCallable)
	bool AddSkills(int32 ToAdd, bool bNotify = true);

	UFUNCTION(BlueprintCallable)
	bool TryTakeSkills(int32 ToTake = 1, bool bNotify = true);

	UFUNCTION(BlueprintCallable)
	bool AccumulateSpentSkills(int32 ToAdd);

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetSkillsCount() const { return Progression.SkillsAvailable; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetSkillsSpentCount() const { return Progression.SkillsSpent; }

	FORCEINLINE int32 GetMaxSkillsAllowed() const { return MaxSkillsAllowed; }

	#pragma endregion

	#pragma region UPGRADES

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsFullyUpgraded() const { return Progression.UpgradesUnlocked == MaxUpgrades; }

	UFUNCTION(BlueprintCallable)
	bool SetUpgradesCount(int32 Value, bool bNotify = true);

	UFUNCTION(BlueprintCallable)
	bool AddUnlokedUpgrades(int32 Value = 1, bool bNotify = true);

	FORCEINLINE int32 GetMaxUpgrades() const { return MaxUpgrades; }
	FORCEINLINE int32 GetUpgradesUnlocked() const { return Progression.UpgradesUnlocked; }

	#pragma endregion

	FORCEINLINE FPlayerProgressionBalance GetProgressionState() const { return Progression; };

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills", meta = (AllowPrivateAccess = true, ClampMin = 1, UIMin = 1))
	int32 MaxSkillsAllowed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrades", meta = (AllowPrivateAccess = true, ClampMin = 1, UIMin = 1))
	int32 MaxUpgrades;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FPlayerProgressionBalance Progression;
};
