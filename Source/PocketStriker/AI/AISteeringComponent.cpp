// Copyright Epic Games, Inc. All Rights Reserved.

#include "AISteeringComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

UAISteeringComponent::UAISteeringComponent()
	: MaxSpeed(600.0f)
	, MaxForce(500.0f)
	, AvoidanceRadius(100.0f)
	, SeparationRadius(150.0f)
	, CurrentVelocity(FVector::ZeroVector)
	, DesiredVelocity(FVector::ZeroVector)
	, CurrentPathIndex(0)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAISteeringComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAISteeringComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update steering forces
	ApplySteeringForce(DeltaTime);
}

FVector UAISteeringComponent::CalculateSeek(const FVector& Target)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FVector::ZeroVector;
	}

	FVector CurrentPosition = Owner->GetActorLocation();
	FVector DesiredDirection = (Target - CurrentPosition).GetSafeNormal();
	DesiredVelocity = DesiredDirection * MaxSpeed;

	// Steering force = desired velocity - current velocity
	FVector SteeringForce = DesiredVelocity - CurrentVelocity;
	
	// Clamp to max force
	if (SteeringForce.Size() > MaxForce)
	{
		SteeringForce = SteeringForce.GetSafeNormal() * MaxForce;
	}

	return SteeringForce;
}

FVector UAISteeringComponent::CalculateAvoidance(const TArray<AActor*>& Obstacles)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FVector::ZeroVector;
	}

	FVector AvoidanceForce = FVector::ZeroVector;
	FVector CurrentPosition = Owner->GetActorLocation();

	for (AActor* Obstacle : Obstacles)
	{
		if (!Obstacle || Obstacle == Owner)
		{
			continue;
		}

		FVector ObstaclePosition = Obstacle->GetActorLocation();
		float Distance = FVector::Dist(CurrentPosition, ObstaclePosition);

		if (Distance < AvoidanceRadius && Distance > 0.0f)
		{
			// Calculate avoidance direction (away from obstacle)
			FVector AwayDirection = (CurrentPosition - ObstaclePosition).GetSafeNormal();
			
			// Stronger force when closer
			float Strength = (AvoidanceRadius - Distance) / AvoidanceRadius;
			AvoidanceForce += AwayDirection * Strength * MaxForce;
		}
	}

	// Clamp total avoidance force
	if (AvoidanceForce.Size() > MaxForce)
	{
		AvoidanceForce = AvoidanceForce.GetSafeNormal() * MaxForce;
	}

	return AvoidanceForce;
}

FVector UAISteeringComponent::CalculateSeparation(const TArray<AActor*>& Neighbors)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FVector::ZeroVector;
	}

	FVector SeparationForce = FVector::ZeroVector;
	FVector CurrentPosition = Owner->GetActorLocation();
	int32 NeighborCount = 0;

	for (AActor* Neighbor : Neighbors)
	{
		if (!Neighbor || Neighbor == Owner)
		{
			continue;
		}

		FVector NeighborPosition = Neighbor->GetActorLocation();
		float Distance = FVector::Dist(CurrentPosition, NeighborPosition);

		if (Distance < SeparationRadius && Distance > 0.0f)
		{
			// Calculate separation direction (away from neighbor)
			FVector AwayDirection = (CurrentPosition - NeighborPosition).GetSafeNormal();
			
			// Stronger force when closer
			float Strength = (SeparationRadius - Distance) / SeparationRadius;
			SeparationForce += AwayDirection * Strength;
			NeighborCount++;
		}
	}

	// Average the separation force
	if (NeighborCount > 0)
	{
		SeparationForce /= NeighborCount;
		SeparationForce = SeparationForce.GetSafeNormal() * MaxForce;
	}

	return SeparationForce;
}

FVector UAISteeringComponent::ComputeSteeringForce()
{
	// This will be called by the AI controller with specific weights
	// For now, return zero - the controller will combine forces manually
	return FVector::ZeroVector;
}

void UAISteeringComponent::ApplySteeringForce(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Get character movement component
	ACharacter* Character = Cast<ACharacter>(Owner);
	if (!Character || !Character->GetCharacterMovement())
	{
		return;
	}

	// Update velocity based on steering force
	FVector SteeringForce = ComputeSteeringForce();
	CurrentVelocity += SteeringForce * DeltaTime;

	// Clamp to max speed
	if (CurrentVelocity.Size() > MaxSpeed)
	{
		CurrentVelocity = CurrentVelocity.GetSafeNormal() * MaxSpeed;
	}

	// Apply velocity to character movement
	if (CurrentVelocity.Size() > 0.1f)
	{
		Character->GetCharacterMovement()->Velocity = CurrentVelocity;
	}
}

void UAISteeringComponent::FollowPath(const FNavPathSharedPtr& Path)
{
	if (!Path.IsValid() || Path->GetPathPoints().Num() == 0)
	{
		CurrentPath = nullptr;
		CurrentPathIndex = 0;
		return;
	}

	CurrentPath = Path;
	CurrentPathIndex = 0;

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Follow path points
	const TArray<FNavPathPoint>& PathPoints = Path->GetPathPoints();
	
	if (CurrentPathIndex < PathPoints.Num())
	{
		FVector TargetPoint = PathPoints[CurrentPathIndex].Location;
		FVector CurrentPosition = Owner->GetActorLocation();
		
		// Check if we've reached current waypoint
		float DistanceToWaypoint = FVector::Dist2D(CurrentPosition, TargetPoint);
		if (DistanceToWaypoint < 100.0f) // Within 1 meter
		{
			CurrentPathIndex++;
		}

		// Calculate seek force to current waypoint
		if (CurrentPathIndex < PathPoints.Num())
		{
			TargetPoint = PathPoints[CurrentPathIndex].Location;
			FVector SeekForce = CalculateSeek(TargetPoint);
			CurrentVelocity += SeekForce * 0.016f; // Approximate delta time
		}
	}
}
