// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_SetLookAtAimOffset.generated.h"


class ACharacter;
class AEnemyCharacter;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UBTDecorator_SetLookAtAimOffset : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_SetLookAtAimOffset(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FString GetStaticDescription() const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintReadWrite, Category = "Blackboard", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector TargetBlackboardKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintReadWrite, Category = "Blackboard", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector TargetVisibilityBlackboardKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintReadWrite, Category = "LookAt", meta = (AllowPrivateAccess = true))
	FName LookFromSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintReadWrite, Category = "LookAt", meta = (AllowPrivateAccess = true))
	FName LookAtSocketName;

	TWeakObjectPtr<ACharacter> TargetCharacter;
	TWeakObjectPtr<AEnemyCharacter> OwnedCharacter;
};
