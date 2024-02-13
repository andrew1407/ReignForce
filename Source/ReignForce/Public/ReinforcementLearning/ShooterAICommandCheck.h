#pragma once

#include "Containers/Set.h"
#include "ReinforcementLearning/ShooterAICommand.h"
#include "ShooterAICommandCheck.generated.h"


USTRUCT(BlueprintType)
struct FShooterAICommandCheck
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSoleComparison = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bSoleComparison", EditConditionHides))
    EShooterAICommand Command = EShooterAICommand::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bSoleComparison", EditConditionHides))
    TSet<EShooterAICommand> Commands;

    bool Check(EShooterAICommand CommandToCheck) const
    {
        return bSoleComparison ? Command == CommandToCheck : Commands.Contains(CommandToCheck);
    }
};