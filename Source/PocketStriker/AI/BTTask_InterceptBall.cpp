// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_InterceptBall.h"
#include "AIControllerFootball.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UBTTask_InterceptBall::UBTTask_InterceptBall()
	: MaxInterceptTime(3.0f)
	, AISpeed(600.0f)
{
	NodeName = "Intercept Ball";
}

EBTNodeResult::Type UBTTask_InterceptBall::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIControllerFootball* AIController = Cast<AAIControllerFootball>(OwnerComp.GetAIOwner());
	if (!AIController || !AIController->GetPawn())
	{
		return EBTNodeResult::Failed;
	}

	// Get ball actor
	UWorld* World = AIController->GetWorld();
	if (!World)
	{
		return EBTNodeResult::Failed;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(World, FName("Ball"), FoundActors);

	if (FoundActors.Num() == 0)
	{
		return EBTNodeResult::Failed;
	}

	AActor* Ball = FoundActors[0];
	FVector BallLocation = Ball->GetActorLocation();
	
	// Get ball velocity (if ball has physics component)
	FVector BallVelocity = FVector::ZeroVector;
	UPrimitiveComponent* BallPrimitive = Cast<UPrimitiveComponent>(Ball->GetRootComponent());
	if (BallPrimitive && BallPrimitive->IsSimulatingPhysics())
	{
		BallVelocity = BallPrimitive->GetPhysicsLinearVelocity();
	}

	// Calculate intercept point
	FVector AILocation = AIController->GetPawn()->GetActorLocation();
	FVector InterceptPoint = CalculateInterceptPoint(AILocation, BallLocation, BallVelocity);

	// Store in blackboard
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsVector(FName("InterceptPoint"), InterceptPoint);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

FVector UBTTask_InterceptBall::PredictBallTrajectory(const FVector& BallLocation, const FVector& BallVelocity, float LookaheadTime)
{
	// Simple ballistic prediction (ignoring air resistance and bounce)
	// Position = InitialPosition + Velocity * Time + 0.5 * Gravity * Time^2
	
	FVector Gravity = FVector(0.0f, 0.0f, -980.0f); // cm/s^2
	FVector PredictedPosition = BallLocation + (BallVelocity * LookaheadTime) + (0.5f * Gravity * LookaheadTime * LookaheadTime);

	// Clamp to ground level (simplified)
	if (PredictedPosition.Z < BallLocation.Z)
	{
		PredictedPosition.Z = BallLocation.Z;
	}

	return PredictedPosition;
}

FVector UBTTask_InterceptBall::CalculateInterceptPoint(const FVector& AILocation, const FVector& BallLocation, const FVector& BallVelocity)
{
	// If ball is stationary or moving very slowly, intercept at current location
	if (BallVelocity.Size() < 50.0f)
	{
		return BallLocation;
	}

	// Iterative approach to find intercept point
	// We need to find time T where:
	// Distance(AI to PredictedBallPos(T)) = AISpeed * T

	float BestTime = 0.0f;
	float BestError = FLT_MAX;
	FVector BestInterceptPoint = BallLocation;

	// Sample different intercept times
	for (float T = 0.1f; T <= MaxInterceptTime; T += 0.1f)
	{
		FVector PredictedBallPos = PredictBallTrajectory(BallLocation, BallVelocity, T);
		float DistanceToIntercept = FVector::Dist(AILocation, PredictedBallPos);
		float TimeToReach = DistanceToIntercept / AISpeed;

		// Calculate error between predicted time and actual time needed
		float Error = FMath::Abs(T - TimeToReach);

		if (Error < BestError)
		{
			BestError = Error;
			BestTime = T;
			BestInterceptPoint = PredictedBallPos;
		}
	}

	// If no good intercept found, just go to current ball location
	if (BestError > 1.0f) // More than 1 second error
	{
		return BallLocation;
	}

	return BestInterceptPoint;
}
