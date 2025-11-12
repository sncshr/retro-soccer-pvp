// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_EvaluateTacticalPosition.h"
#include "AIControllerFootball.h"
#include "FootballAIUtility.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

UBTTask_EvaluateTacticalPosition::UBTTask_EvaluateTacticalPosition()
	: MinDistanceFromBall(300.0f)
	, MaxDistanceFromBall(800.0f)
	, MinDistanceFromOpponents(200.0f)
{
	NodeName = "Evaluate Tactical Position";
}

EBTNodeResult::Type UBTTask_EvaluateTacticalPosition::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIControllerFootball* AIController = Cast<AAIControllerFootball>(OwnerComp.GetAIOwner());
	if (!AIController || !AIController->GetPawn())
	{
		return EBTNodeResult::Failed;
	}

	// Create utility system to build context
	UFootballAIUtility* UtilitySystem = NewObject<UFootballAIUtility>();
	if (!UtilitySystem)
	{
		return EBTNodeResult::Failed;
	}

	// Build tactical context
	FAIContext Context = UtilitySystem->BuildContext(AIController);

	// Calculate optimal position
	FVector OptimalPosition = CalculateOptimalPosition(Context);

	// Store in blackboard
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsVector(FName("TacticalPosition"), OptimalPosition);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

FVector UBTTask_EvaluateTacticalPosition::CalculateOptimalPosition(const FAIContext& Context)
{
	// Start with current AI position
	FVector OptimalPosition = Context.AIPosition;

	// If ball location is unknown, stay in current position
	if (Context.BallLocation.IsNearlyZero())
	{
		return OptimalPosition;
	}

	// Calculate direction from ball to AI
	FVector BallToAI = (Context.AIPosition - Context.BallLocation).GetSafeNormal();

	// Calculate base position at optimal distance from ball
	float TargetDistance = (MinDistanceFromBall + MaxDistanceFromBall) * 0.5f;
	FVector BasePosition = Context.BallLocation + (BallToAI * TargetDistance);

	// Adjust position to maintain spacing from opponents
	FVector AdjustmentVector = FVector::ZeroVector;
	int32 AdjustmentCount = 0;

	for (const FVector& OpponentPos : Context.OpponentPositions)
	{
		float DistanceToOpponent = FVector::Dist(BasePosition, OpponentPos);
		
		if (DistanceToOpponent < MinDistanceFromOpponents)
		{
			// Push away from opponent
			FVector AwayFromOpponent = (BasePosition - OpponentPos).GetSafeNormal();
			float PushStrength = (MinDistanceFromOpponents - DistanceToOpponent) / MinDistanceFromOpponents;
			AdjustmentVector += AwayFromOpponent * PushStrength * 200.0f;
			AdjustmentCount++;
		}
	}

	// Apply average adjustment
	if (AdjustmentCount > 0)
	{
		AdjustmentVector /= AdjustmentCount;
		OptimalPosition = BasePosition + AdjustmentVector;
	}
	else
	{
		OptimalPosition = BasePosition;
	}

	// Ensure position is within valid distance range from ball
	float DistanceToBall = FVector::Dist(OptimalPosition, Context.BallLocation);
	if (DistanceToBall < MinDistanceFromBall)
	{
		FVector Direction = (OptimalPosition - Context.BallLocation).GetSafeNormal();
		OptimalPosition = Context.BallLocation + (Direction * MinDistanceFromBall);
	}
	else if (DistanceToBall > MaxDistanceFromBall)
	{
		FVector Direction = (OptimalPosition - Context.BallLocation).GetSafeNormal();
		OptimalPosition = Context.BallLocation + (Direction * MaxDistanceFromBall);
	}

	return OptimalPosition;
}
