// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NavigationPath.h"
#include "AISteeringComponent.generated.h"

/**
 * AI steering component for movement behaviors
 * Implements steering forces for seek, avoidance, and separation
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POCKETSTRIKER_API UAISteeringComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAISteeringComponent();

	// Steering behaviors
	FVector CalculateSeek(const FVector& Target);
	FVector CalculateAvoidance(const TArray<AActor*>& Obstacles);
	FVector CalculateSeparation(const TArray<AActor*>& Neighbors);
	
	// Combined steering
	FVector ComputeSteeringForce();
	void ApplySteeringForce(float DeltaTime);
	
	// NavMesh integration
	void FollowPath(const FNavPathSharedPtr& Path);

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float MaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float MaxForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float AvoidanceRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float SeparationRadius;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FVector CurrentVelocity;
	FVector DesiredVelocity;
	FNavPathSharedPtr CurrentPath;
	int32 CurrentPathIndex;
};
