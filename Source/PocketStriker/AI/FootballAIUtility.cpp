// Copyright Epic Games, Inc. All Rights Reserved.

#include "FootballAIUtility.h"
#include "AIControllerFootball.h"
#include "GameFramework/Pawn.h"

UFootballAIUtility::UFootballAIUtility()
{
}

FAIContext UFootballAIUtility::BuildContext(AAIControllerFootball* Controller)
{
	FAIContext Context;

	if (!Controller || !Controller->GetPawn())
	{
		return Context;
	}

	// Get AI position
	Context.AIPosition = Controller->GetPawn()->GetActorLocation();

	// Get ball location
	Context.BallLocation = Controller->LastKnownBallLocation;

	// Calculate distance to ball
	Context.DistanceToBall = FVector::Dist(Context.AIPosition, Context.BallLocation);

	// Get nearby opponent positions
	TArray<AActor*> Opponents = Controller->GetNearbyOpponents(2000.0f);
	for (AActor* Opponent : Opponents)
	{
		if (Opponent)
		{
			Context.OpponentPositions.Add(Opponent->GetActorLocation());
		}
	}

	return Context;
}

float UFootballAIUtility::ScorePress(const FAIContext& Context)
{
	// Press behavior: aggressively pursue the ball
	// Higher score when:
	// - Close to ball
	// - Few opponents nearby
	// - Ball is not too far away

	if (Context.BallLocation.IsNearlyZero())
	{
		return 0.0f;
	}

	float Score = 0.0f;

	// Distance factor (closer = higher score)
	float MaxPressDistance = 1000.0f;
	if (Context.DistanceToBall < MaxPressDistance)
	{
		float DistanceFactor = 1.0f - (Context.DistanceToBall / MaxPressDistance);
		Score += DistanceFactor * 0.6f; // 60% weight
	}

	// Opponent proximity factor (fewer nearby = higher score)
	int32 NearbyOpponents = 0;
	for (const FVector& OpponentPos : Context.OpponentPositions)
	{
		float DistanceToOpponent = FVector::Dist(Context.AIPosition, OpponentPos);
		if (DistanceToOpponent < 500.0f)
		{
			NearbyOpponents++;
		}
	}

	float OpponentFactor = FMath::Clamp(1.0f - (NearbyOpponents * 0.3f), 0.0f, 1.0f);
	Score += OpponentFactor * 0.4f; // 40% weight

	return FMath::Clamp(Score, 0.0f, 1.0f);
}

float UFootballAIUtility::ScoreSupport(const FAIContext& Context)
{
	// Support behavior: position for receiving a pass
	// Higher score when:
	// - Medium distance from ball (not too close, not too far)
	// - Good spacing from teammates
	// - Open passing lane

	if (Context.BallLocation.IsNearlyZero())
	{
		return 0.0f;
	}

	float Score = 0.0f;

	// Optimal support distance (300-800 units)
	float OptimalMinDistance = 300.0f;
	float OptimalMaxDistance = 800.0f;

	if (Context.DistanceToBall >= OptimalMinDistance && Context.DistanceToBall <= OptimalMaxDistance)
	{
		// In optimal range
		Score += 0.7f;
	}
	else if (Context.DistanceToBall < OptimalMinDistance)
	{
		// Too close
		float Penalty = (OptimalMinDistance - Context.DistanceToBall) / OptimalMinDistance;
		Score += 0.7f * (1.0f - Penalty);
	}
	else
	{
		// Too far
		float Penalty = (Context.DistanceToBall - OptimalMaxDistance) / 1000.0f;
		Score += 0.7f * FMath::Clamp(1.0f - Penalty, 0.0f, 1.0f);
	}

	// Spacing factor (good spacing = higher score)
	float MinSpacing = 200.0f;
	bool bHasGoodSpacing = true;
	for (const FVector& OpponentPos : Context.OpponentPositions)
	{
		float Distance = FVector::Dist(Context.AIPosition, OpponentPos);
		if (Distance < MinSpacing)
		{
			bHasGoodSpacing = false;
			break;
		}
	}

	if (bHasGoodSpacing)
	{
		Score += 0.3f;
	}

	return FMath::Clamp(Score, 0.0f, 1.0f);
}

float UFootballAIUtility::ScoreIntercept(const FAIContext& Context)
{
	// Intercept behavior: cut off passing lanes and ball movement
	// Higher score when:
	// - Ball is moving (would need velocity data)
	// - AI is positioned between ball and opponent
	// - Medium distance from ball

	if (Context.BallLocation.IsNearlyZero())
	{
		return 0.0f;
	}

	float Score = 0.0f;

	// Distance factor (medium distance preferred)
	float OptimalDistance = 500.0f;
	float DistanceDiff = FMath::Abs(Context.DistanceToBall - OptimalDistance);
	float DistanceFactor = FMath::Clamp(1.0f - (DistanceDiff / 1000.0f), 0.0f, 1.0f);
	Score += DistanceFactor * 0.5f;

	// Positioning factor (between ball and opponents)
	if (Context.OpponentPositions.Num() > 0)
	{
		// Check if AI is between ball and any opponent
		for (const FVector& OpponentPos : Context.OpponentPositions)
		{
			FVector BallToOpponent = OpponentPos - Context.BallLocation;
			FVector BallToAI = Context.AIPosition - Context.BallLocation;

			// Dot product to check if AI is in the direction of opponent from ball
			float DotProduct = FVector::DotProduct(BallToOpponent.GetSafeNormal(), BallToAI.GetSafeNormal());
			if (DotProduct > 0.5f) // AI is roughly between ball and opponent
			{
				Score += 0.5f;
				break;
			}
		}
	}

	return FMath::Clamp(Score, 0.0f, 1.0f);
}

float UFootballAIUtility::ScoreCover(const FAIContext& Context)
{
	// Cover behavior: defensive positioning to mark opponents
	// Higher score when:
	// - Opponents are nearby
	// - Ball is far away
	// - Need to maintain defensive shape

	float Score = 0.0f;

	// Distance to ball factor (farther = higher score for covering)
	float MinCoverDistance = 800.0f;
	if (Context.DistanceToBall > MinCoverDistance)
	{
		float DistanceFactor = FMath::Clamp((Context.DistanceToBall - MinCoverDistance) / 1000.0f, 0.0f, 1.0f);
		Score += DistanceFactor * 0.6f;
	}

	// Opponent proximity factor (opponents nearby = higher score)
	int32 NearbyOpponents = 0;
	for (const FVector& OpponentPos : Context.OpponentPositions)
	{
		float DistanceToOpponent = FVector::Dist(Context.AIPosition, OpponentPos);
		if (DistanceToOpponent < 600.0f)
		{
			NearbyOpponents++;
		}
	}

	if (NearbyOpponents > 0)
	{
		float OpponentFactor = FMath::Clamp(NearbyOpponents * 0.3f, 0.0f, 1.0f);
		Score += OpponentFactor * 0.4f;
	}

	return FMath::Clamp(Score, 0.0f, 1.0f);
}

float UFootballAIUtility::ScoreRetreat(const FAIContext& Context)
{
	// Retreat behavior: fall back to defensive position
	// Higher score when:
	// - Ball is very far away
	// - Many opponents nearby
	// - Under pressure

	float Score = 0.0f;

	// Distance to ball factor (very far = higher score)
	float MinRetreatDistance = 1500.0f;
	if (Context.DistanceToBall > MinRetreatDistance)
	{
		float DistanceFactor = FMath::Clamp((Context.DistanceToBall - MinRetreatDistance) / 1000.0f, 0.0f, 1.0f);
		Score += DistanceFactor * 0.5f;
	}

	// Pressure factor (many nearby opponents = higher score)
	int32 NearbyOpponents = 0;
	for (const FVector& OpponentPos : Context.OpponentPositions)
	{
		float DistanceToOpponent = FVector::Dist(Context.AIPosition, OpponentPos);
		if (DistanceToOpponent < 400.0f)
		{
			NearbyOpponents++;
		}
	}

	if (NearbyOpponents >= 2)
	{
		Score += 0.5f; // High pressure situation
	}

	return FMath::Clamp(Score, 0.0f, 1.0f);
}
