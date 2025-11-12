// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_InterceptBall.generated.h"

/**
 * Behavior tree task for intercepting the ball
 * Predicts ball trajectory and calculates intercept point
 */
UCLASS()
class POCKETSTRIKER_API UBTTask_InterceptBall : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_InterceptBall();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// Ball trajectory prediction
	FVector PredictBallTrajectory(const FVector& BallLocation, const FVector& BallVelocity, float LookaheadTime);
	FVector CalculateInterceptPoint(const FVector& AILocation, const FVector& BallLocation, const FVector& BallVelocity);

protected:
	UPROPERTY(EditAnywhere, Category = "Intercept")
	float MaxInterceptTime;

	UPROPERTY(EditAnywhere, Category = "Intercept")
	float AISpeed;
};
