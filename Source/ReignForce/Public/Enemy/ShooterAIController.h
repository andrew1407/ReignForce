// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ReinforcementLearning/ShooterAICommand.h"
#include "ReinforcementLearning/TemperamentType.h"
#include "ReinforcementLearning/Interfaces/BehaviourIdentity.h"
#include "ShooterAIController.generated.h"


class UBehaviorTree;
class UBehaviorTreeComponent;

class UAISense;

class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;

class AShooterCharacter;

DECLARE_DELEGATE_OneParam(FSenseHandle, const FActorPerceptionUpdateInfo&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTargetVisibilityChanged, bool, bState, AShooterCharacter*, ShooterSelf, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTargetFocusChanged, AShooterCharacter*, ShooterSelf, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnShooterAICommandChanged, EShooterAICommand, Command, EShooterAICommand, PreviousCommand, AShooterCharacter*, ShooterSelf);

/**
 * 
 */
UCLASS()
class REIGNFORCE_API AShooterAIController : public AAIController, public IBehaviourIdentity
{
	GENERATED_BODY()

public:
	AShooterAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	#pragma region EVENTS

	UPROPERTY(BlueprintAssignable)
	FOnTargetVisibilityChanged OnTargetVisibilityChanged;

	UPROPERTY(BlueprintAssignable)
	FOnShooterAICommandChanged OnShooterAICommandChanged;

	UPROPERTY(BlueprintAssignable)
	FOnTargetFocusChanged OnTargetFocusChanged;

	#pragma endregion

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SenseOptions", meta = (ClampMin = 0, UIMin = 0))
	float IgnoreAllyDistanceHearing;

	virtual FGenericTeamId GetGenericTeamId() const override;

	FORCEINLINE virtual ETemperamentType GetTemperamentType_Implementation() override { return TemperamentType; }
	FORCEINLINE virtual void ChangeTemperamentType_Implementation(ETemperamentType TemperamentToSet) override { TemperamentType = TemperamentToSet; }

	UFUNCTION(BlueprintCallable)
	EShooterAICommand GetShooterAICommand() const;

	UFUNCTION(BlueprintCallable)
	void SetShooterAICommand(EShooterAICommand Command);

	UFUNCTION(BlueprintCallable)
	void StopCommandsFetchTimer();

	UFUNCTION(BlueprintPure)
	AActor* GetTargetFocus() const;

	UFUNCTION(BlueprintCallable)
	void ResetTargetFocus(AActor* Target, bool bApplyToTargetVisibility = true);

	UFUNCTION(BlueprintCallable)
	void SetTargetVisibility(bool bState);

protected:
	virtual void BeginDestroy() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

private:
	#pragma region COMPONENTS

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BehaviourTree", meta = (AllowPrivateAccess = true))
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BehaviourTree", meta = (AllowPrivateAccess = true))
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	#pragma endregion

	#pragma region BLACKBOARD_KEYS

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard|Keys", meta = (AllowPrivateAccess = true))
	FName FocusedTargetKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard|Keys", meta = (AllowPrivateAccess = true))
	FName PositionToInvestigateKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard|Keys", meta = (AllowPrivateAccess = true))
	FName MemoizedSenseKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard|Keys", meta = (AllowPrivateAccess = true))
	FName IsTargetVisibleKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard|Keys", meta = (AllowPrivateAccess = true))
	FName LastSeenTargetPositionKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard|Keys", meta = (AllowPrivateAccess = true))
    FName CombatCommandKey;

	#pragma endregion

	#pragma region SENSE_CONFIGS

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;

	#pragma endregion

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Temperament", meta = (AllowPrivateAccess = true))
	ETemperamentType TemperamentType;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TWeakObjectPtr<AShooterCharacter> PossessedShooter;

	UPROPERTY()
	FString WeaponNoiseTagToIgnore;

	UPROPERTY()
	FTimerHandle SightTimerHandle;

	UPROPERTY()
	FTimerHandle AICommandTimerHandle;

	TMap<TSubclassOf<UAISense>, FSenseHandle> SenseHandlers;

	UFUNCTION()
	void OnPerceptionInfoUpdated(const FActorPerceptionUpdateInfo& UpdateInfo);

	UFUNCTION()
	void TriggerAICommandsFetch(bool bState, AShooterCharacter* ShooterSelf, AActor* Target);

	UFUNCTION()
	void HandleShooterTargetVisibility(bool bState, AShooterCharacter* ShooterSelf, AActor* Target);

	UFUNCTION()
	void HandleShooterPerish(AShooterCharacter* ShooterSelf, AActor* Cause);

	UFUNCTION()
	void HandleAppliedDamage(float Damage, AShooterCharacter* ShooterSelf, AActor* HitTaret, bool bLethal);

	void ReactToSight(const FActorPerceptionUpdateInfo& UpdateInfo);
	void ReactToHearing(const FActorPerceptionUpdateInfo& UpdateInfo);
	void ReactToDamage(const FActorPerceptionUpdateInfo& UpdateInfo);
};
