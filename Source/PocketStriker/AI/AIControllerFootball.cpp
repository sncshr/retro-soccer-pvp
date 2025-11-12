// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIControllerFootball.h"
#include "AIPerceptionSystem.h"
#include "FootballAIUtility.h"
#include "AIParametersData.h"
#include "AISteeringComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

AAIControllerFootball::AAIControllerFootball()
	: PerceptionRadius(2000.0f)
	, LastKnownBallLocation(FVector::ZeroVector)
	, CurrentBehavior(EAIBehavior::Idle)
	, PerceptionUpdateInterval(0.2f) // Update perception every 200ms
	, DecisionUpdateInterval(0.3f) // Update decisions every 300ms
	, TimeSinceLastPerceptionUpdate(0.0f)
	, TimeSinceLastDecisionUpdate(0.0f)
	, UpdateTimeOffset(0.0f)
{
	PrimaryActorTick.bCanEverTick = true;

	// Stagger updates across AI agents to spread load
	UpdateTimeOffset = FMath::FRandRange(0.0f, 0.1f);

	// Create perception component
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	
	// Configure sight sense
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if (SightConfig)
	{
		SightConfig->SightRadius = 2000.0f;
		SightConfig->LoseSightRadius = 2500.0f;
		SightConfig->PeripheralVisionAngleDegrees = 180.0f;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

		PerceptionComponent->ConfigureSense(*SightConfig);
		PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	}

	// Create behavior tree component
	BehaviorTree = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));

	// Create steering component
	SteeringComponent = CreateDefaultSubobject<UAISteeringComponent>(TEXT("SteeringComponent"));
}

void AAIControllerFootball::BeginPlay()
{
	Super::BeginPlay();

	// Initialize perception system
	if (PerceptionComponent)
	{
		PerceptionComponent->Activate();
	}
}

void AAIControllerFootball::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Throttle perception updates with staggered timing
	TimeSinceLastPerceptionUpdate += DeltaTime;
	if (TimeSinceLastPerceptionUpdate >= (PerceptionUpdateInterval + UpdateTimeOffset))
	{
		UpdatePerception(DeltaTime);
		TimeSinceLastPerceptionUpdate = 0.0f;
	}

	// Throttle decision updates
	TimeSinceLastDecisionUpdate += DeltaTime;
	if (TimeSinceLastDecisionUpdate >= DecisionUpdateInterval)
	{
		EvaluateUtilityScores();
		TimeSinceLastDecisionUpdate = 0.0f;
	}

	// Execute current behavior
	ExecuteBehavior(CurrentBehavior, DeltaTime);

	// Draw debug info
	DrawDebugInfo();
}

void AAIControllerFootball::UpdatePerception(float DeltaTime)
{
	if (!GetPawn())
	{
		return;
	}

	FVector AILocation = GetPawn()->GetActorLocation();

	// Try to detect ball
	UAIPerceptionSystem* PerceptionSystem = NewObject<UAIPerceptionSystem>(this);
	if (PerceptionSystem)
	{
		FVector BallLocation;
		if (PerceptionSystem->DetectBall(AILocation, PerceptionRadius, BallLocation))
		{
			LastKnownBallLocation = BallLocation;
		}
	}

	// Evaluate utility scores and select behavior
	EvaluateUtilityScores();
}

bool AAIControllerFootball::CanSeeBall() const
{
	if (!GetPawn())
	{
		return false;
	}

	FVector AILocation = GetPawn()->GetActorLocation();
	
	// Check if we have a recent ball location
	if (LastKnownBallLocation.IsNearlyZero())
	{
		return false;
	}

	// Check if ball is within perception radius
	float DistanceToBall = FVector::Dist(AILocation, LastKnownBallLocation);
	if (DistanceToBall > PerceptionRadius)
	{
		return false;
	}

	// Check line of sight
	UAIPerceptionSystem* PerceptionSystem = NewObject<UAIPerceptionSystem>(this);
	if (PerceptionSystem)
	{
		return PerceptionSystem->HasLineOfSight(AILocation, LastKnownBallLocation);
	}

	return false;
}

TArray<AActor*> AAIControllerFootball::GetNearbyOpponents(float Radius) const
{
	TArray<AActor*> Opponents;

	if (!GetPawn())
	{
		return Opponents;
	}

	FVector AILocation = GetPawn()->GetActorLocation();

	// Use perception system for spatial query
	UAIPerceptionSystem* PerceptionSystem = NewObject<UAIPerceptionSystem>(this);
	if (PerceptionSystem)
	{
		PerceptionSystem->BuildProximityGrid();
		TArray<AActor*> NearbyActors = PerceptionSystem->QueryNearbyActors(AILocation, Radius);

		// Filter for opponents (actors with Player tag)
		for (AActor* Actor : NearbyActors)
		{
			if (Actor && Actor->ActorHasTag(FName("Player")))
			{
				Opponents.Add(Actor);
			}
		}
	}

	return Opponents;
}

void AAIControllerFootball::EvaluateUtilityScores()
{
	if (!GetPawn())
	{
		return;
	}

	// Create utility system
	UFootballAIUtility* UtilitySystem = NewObject<UFootballAIUtility>(this);
	if (!UtilitySystem)
	{
		return;
	}

	// Build context
	FAIContext Context = UtilitySystem->BuildContext(this);

	// Score all behaviors
	TArray<FUtilityScore> Scores;

	FUtilityScore PressScore;
	PressScore.Behavior = EAIBehavior::Press;
	PressScore.Score = UtilitySystem->ScorePress(Context);
	PressScore.Reasoning = FString::Printf(TEXT("Press: %.2f"), PressScore.Score);
	Scores.Add(PressScore);

	FUtilityScore SupportScore;
	SupportScore.Behavior = EAIBehavior::Support;
	SupportScore.Score = UtilitySystem->ScoreSupport(Context);
	SupportScore.Reasoning = FString::Printf(TEXT("Support: %.2f"), SupportScore.Score);
	Scores.Add(SupportScore);

	FUtilityScore InterceptScore;
	InterceptScore.Behavior = EAIBehavior::Intercept;
	InterceptScore.Score = UtilitySystem->ScoreIntercept(Context);
	InterceptScore.Reasoning = FString::Printf(TEXT("Intercept: %.2f"), InterceptScore.Score);
	Scores.Add(InterceptScore);

	FUtilityScore CoverScore;
	CoverScore.Behavior = EAIBehavior::Cover;
	CoverScore.Score = UtilitySystem->ScoreCover(Context);
	CoverScore.Reasoning = FString::Printf(TEXT("Cover: %.2f"), CoverScore.Score);
	Scores.Add(CoverScore);

	FUtilityScore RetreatScore;
	RetreatScore.Behavior = EAIBehavior::Retreat;
	RetreatScore.Score = UtilitySystem->ScoreRetreat(Context);
	RetreatScore.Reasoning = FString::Printf(TEXT("Retreat: %.2f"), RetreatScore.Score);
	Scores.Add(RetreatScore);

	// Select best behavior
	CurrentBehavior = SelectBestBehavior();
}

EAIBehavior AAIControllerFootball::SelectBestBehavior()
{
	if (!GetPawn())
	{
		return EAIBehavior::Idle;
	}

	// Create utility system
	UFootballAIUtility* UtilitySystem = NewObject<UFootballAIUtility>(this);
	if (!UtilitySystem)
	{
		return EAIBehavior::Idle;
	}

	// Build context
	FAIContext Context = UtilitySystem->BuildContext(this);

	// Score all behaviors and find best
	float BestScore = -1.0f;
	EAIBehavior BestBehavior = EAIBehavior::Idle;

	float PressScore = UtilitySystem->ScorePress(Context);
	if (PressScore > BestScore)
	{
		BestScore = PressScore;
		BestBehavior = EAIBehavior::Press;
	}

	float SupportScore = UtilitySystem->ScoreSupport(Context);
	if (SupportScore > BestScore)
	{
		BestScore = SupportScore;
		BestBehavior = EAIBehavior::Support;
	}

	float InterceptScore = UtilitySystem->ScoreIntercept(Context);
	if (InterceptScore > BestScore)
	{
		BestScore = InterceptScore;
		BestBehavior = EAIBehavior::Intercept;
	}

	float CoverScore = UtilitySystem->ScoreCover(Context);
	if (CoverScore > BestScore)
	{
		BestScore = CoverScore;
		BestBehavior = EAIBehavior::Cover;
	}

	float RetreatScore = UtilitySystem->ScoreRetreat(Context);
	if (RetreatScore > BestScore)
	{
		BestScore = RetreatScore;
		BestBehavior = EAIBehavior::Retreat;
	}

	return BestBehavior;
}

void AAIControllerFootball::ExecuteBehavior(EAIBehavior Behavior, float DeltaTime)
{
	if (!GetPawn() || !SteeringComponent)
	{
		return;
	}

	FVector AILocation = GetPawn()->GetActorLocation();
	TArray<AActor*> NearbyActors = GetNearbyOpponents(500.0f);

	// Calculate steering forces based on behavior
	FVector SeekForce = FVector::ZeroVector;
	FVector AvoidanceForce = SteeringComponent->CalculateAvoidance(NearbyActors);
	FVector SeparationForce = SteeringComponent->CalculateSeparation(NearbyActors);

	switch (Behavior)
	{
	case EAIBehavior::Press:
		// Aggressively move towards ball
		if (!LastKnownBallLocation.IsNearlyZero())
		{
			SeekForce = SteeringComponent->CalculateSeek(LastKnownBallLocation);
			
			// Use NavMesh for pathfinding
			UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
			if (NavSys)
			{
				FNavPathSharedPtr Path = NavSys->FindPathToLocationSynchronously(
					GetWorld(),
					AILocation,
					LastKnownBallLocation,
					this
				);
				
				if (Path.IsValid())
				{
					SteeringComponent->FollowPath(Path);
				}
			}
			
			// Combine forces: high seek, low avoidance
			FVector CombinedForce = SeekForce * 0.8f + AvoidanceForce * 0.2f;
			MoveToLocation(LastKnownBallLocation, 50.0f);
		}
		break;

	case EAIBehavior::Support:
		// Position for pass - stay near ball but not too close
		if (!LastKnownBallLocation.IsNearlyZero())
		{
			FVector SupportPosition = LastKnownBallLocation + FVector(300.0f, 300.0f, 0.0f);
			SeekForce = SteeringComponent->CalculateSeek(SupportPosition);
			
			// Combine forces: medium seek, medium separation
			FVector CombinedForce = SeekForce * 0.5f + SeparationForce * 0.5f;
			MoveToLocation(SupportPosition, 100.0f);
		}
		break;

	case EAIBehavior::Intercept:
		// Move to intercept ball path
		if (!LastKnownBallLocation.IsNearlyZero())
		{
			// Calculate intercept point (simplified - assumes ball is stationary)
			FVector InterceptPoint = LastKnownBallLocation;
			SeekForce = SteeringComponent->CalculateSeek(InterceptPoint);
			
			// Combine forces: high seek, medium avoidance
			FVector CombinedForce = SeekForce * 0.7f + AvoidanceForce * 0.3f;
			MoveToLocation(InterceptPoint, 50.0f);
		}
		break;

	case EAIBehavior::Cover:
		// Defensive positioning
		{
			FVector DefensivePosition = AILocation + FVector(-200.0f, 0.0f, 0.0f);
			SeekForce = SteeringComponent->CalculateSeek(DefensivePosition);
			
			// Combine forces: medium seek, high separation
			FVector CombinedForce = SeekForce * 0.4f + SeparationForce * 0.6f;
			MoveToLocation(DefensivePosition, 100.0f);
		}
		break;

	case EAIBehavior::Retreat:
		// Fall back to defensive position
		{
			FVector RetreatPosition = AILocation + FVector(-500.0f, 0.0f, 0.0f);
			SeekForce = SteeringComponent->CalculateSeek(RetreatPosition);
			
			// Combine forces: high seek, low avoidance
			FVector CombinedForce = SeekForce * 0.7f + AvoidanceForce * 0.3f;
			MoveToLocation(RetreatPosition, 100.0f);
		}
		break;

	case EAIBehavior::Idle:
	default:
		// Do nothing
		StopMovement();
		break;
	}
}

void AAIControllerFootball::DrawDebugInfo()
{
	if (!GetPawn())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector AILocation = GetPawn()->GetActorLocation();

	// Draw perception radius
	DrawDebugSphere(World, AILocation, PerceptionRadius, 32, FColor::Yellow, false, 0.1f, 0, 2.0f);

	// Draw line to ball
	if (!LastKnownBallLocation.IsNearlyZero())
	{
		DrawDebugLine(World, AILocation, LastKnownBallLocation, FColor::Green, false, 0.1f, 0, 3.0f);
		
		// Draw ball location marker
		DrawDebugSphere(World, LastKnownBallLocation, 30.0f, 12, FColor::Green, false, 0.1f, 0, 2.0f);
	}

	// Draw line of sight check
	if (CanSeeBall())
	{
		DrawDebugLine(World, AILocation + FVector(0, 0, 80.0f), LastKnownBallLocation, FColor::Cyan, false, 0.1f, 0, 1.0f);
	}

	// Draw current behavior text above AI
	FString BehaviorText;
	FColor BehaviorColor = FColor::White;
	
	switch (CurrentBehavior)
	{
	case EAIBehavior::Press:
		BehaviorText = TEXT("PRESS");
		BehaviorColor = FColor::Red;
		break;
	case EAIBehavior::Support:
		BehaviorText = TEXT("SUPPORT");
		BehaviorColor = FColor::Blue;
		break;
	case EAIBehavior::Intercept:
		BehaviorText = TEXT("INTERCEPT");
		BehaviorColor = FColor::Orange;
		break;
	case EAIBehavior::Cover:
		BehaviorText = TEXT("COVER");
		BehaviorColor = FColor::Purple;
		break;
	case EAIBehavior::Retreat:
		BehaviorText = TEXT("RETREAT");
		BehaviorColor = FColor::Yellow;
		break;
	case EAIBehavior::Idle:
	default:
		BehaviorText = TEXT("IDLE");
		BehaviorColor = FColor::White;
		break;
	}

	DrawDebugString(World, AILocation + FVector(0, 0, 120.0f), BehaviorText, nullptr, BehaviorColor, 0.1f, true, 1.5f);

	// Draw target movement line based on behavior
	FVector TargetLocation = FVector::ZeroVector;
	bool bHasTarget = false;

	switch (CurrentBehavior)
	{
	case EAIBehavior::Press:
		if (!LastKnownBallLocation.IsNearlyZero())
		{
			TargetLocation = LastKnownBallLocation;
			bHasTarget = true;
		}
		break;
	case EAIBehavior::Support:
		if (!LastKnownBallLocation.IsNearlyZero())
		{
			TargetLocation = LastKnownBallLocation + FVector(300.0f, 300.0f, 0.0f);
			bHasTarget = true;
		}
		break;
	case EAIBehavior::Intercept:
		if (!LastKnownBallLocation.IsNearlyZero())
		{
			TargetLocation = LastKnownBallLocation;
			bHasTarget = true;
		}
		break;
	}

	if (bHasTarget)
	{
		DrawDebugLine(World, AILocation, TargetLocation, FColor::Magenta, false, 0.1f, 0, 2.0f);
		DrawDebugSphere(World, TargetLocation, 40.0f, 12, FColor::Magenta, false, 0.1f, 0, 1.0f);
	}

	// Draw predicted ball trajectory if ball is moving
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(World, FName("Ball"), FoundActors);
	
	if (FoundActors.Num() > 0)
	{
		AActor* Ball = FoundActors[0];
		UPrimitiveComponent* BallPrimitive = Cast<UPrimitiveComponent>(Ball->GetRootComponent());
		
		if (BallPrimitive && BallPrimitive->IsSimulatingPhysics())
		{
			FVector BallVelocity = BallPrimitive->GetPhysicsLinearVelocity();
			
			if (BallVelocity.Size() > 50.0f)
			{
				// Draw trajectory prediction
				FVector CurrentPos = Ball->GetActorLocation();
				FVector Gravity = FVector(0.0f, 0.0f, -980.0f);
				
				for (float T = 0.1f; T <= 2.0f; T += 0.1f)
				{
					FVector NextPos = Ball->GetActorLocation() + (BallVelocity * T) + (0.5f * Gravity * T * T);
					
					// Clamp to ground
					if (NextPos.Z < Ball->GetActorLocation().Z)
					{
						NextPos.Z = Ball->GetActorLocation().Z;
					}
					
					DrawDebugLine(World, CurrentPos, NextPos, FColor::Cyan, false, 0.1f, 0, 1.0f);
					CurrentPos = NextPos;
				}
			}
		}
	}

	// Draw nearby opponents
	TArray<AActor*> NearbyOpponents = GetNearbyOpponents(PerceptionRadius);
	for (AActor* Opponent : NearbyOpponents)
	{
		if (Opponent)
		{
			FVector OpponentLocation = Opponent->GetActorLocation();
			DrawDebugLine(World, AILocation, OpponentLocation, FColor::Red, false, 0.1f, 0, 1.0f);
		}
	}
}
