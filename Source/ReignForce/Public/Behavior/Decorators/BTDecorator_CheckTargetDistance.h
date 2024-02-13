// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "Weapons/WeaponType.h"
#include "BTDecorator_CheckTargetDistance.generated.h"


class AShooterCharacter;

USTRUCT(BlueprintType)
struct FDistanceBounds
{
    GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Lower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Upper;
};

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UBTDecorator_CheckTargetDistance : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CheckTargetDistance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FString GetStaticDescription() const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector TargetBlackboardKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector DistanceStateBlackboardKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounds", meta = (AllowPrivateAccess = true))
	TMap<EWeaponType, FDistanceBounds> Distances;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounds", meta = (AllowPrivateAccess = true))
	FDistanceBounds DefaultDistance;

	UPROPERTY()
	TWeakObjectPtr<AShooterCharacter> ControlledCharacter;

	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;
};
