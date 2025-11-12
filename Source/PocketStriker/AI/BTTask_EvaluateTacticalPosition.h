// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "FootballAIUtility.h"
#include "BTTask_EvaluateTacticalPosition.generated.h"

/**
 * Behavior tree task for evaluating and moving to tactical positions
 * Calculates optimal positioning based on ball location and opponent positions
 */
UCLASS()
class POCKETSTRIKER_API UBTTask_EvaluateTacticalPosition : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_EvaluateTacticalPosition();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// Tactical evaluation
	FVector CalculateOptimalPosition(const FAIContext& Context);

protected:
	UPROPERTY(EditAnywhere, Category = "Tactical")
	float MinDistanceFromBall;

	UPROPERTY(EditAnywhere, Category = "Tactical")
	float MaxDistanceFromBall;

	UPROPERTY(EditAnywhere, Category = "Tactical")
	float MinDistanceFromOpponents;
};
