// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_SetRotationFocus.generated.h"


class ACharacter;

/**
 * 
 */
UCLASS()
class REIGNFORCE_API UBTDecorator_SetRotationFocus : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_SetRotationFocus(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual FString GetStaticDescription() const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector BlackboardKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LookAt", meta = (AllowPrivateAccess = true))
	FName LookFromSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LookAt", meta = (AllowPrivateAccess = true))
	FName LookAtSocketName;

	TWeakObjectPtr<ACharacter> Target;
};
