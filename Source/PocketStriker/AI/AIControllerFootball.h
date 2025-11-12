// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIControllerFootball.generated.h"

class UAIPerceptionComponent;
class UBehaviorTreeComponent;
class UAISteeringComponent;

UENUM(BlueprintType)
enum class EAIBehavior : uint8
{
	Idle,
	Press,
	Support,
	Intercept,
	Cover,
	Retreat
};

/**
 * AI controller for football opponents
 * Implements perception, decision making, and behavior execution
 */
UCLASS()
class POCKETSTRIKER_API AAIControllerFootball : public AAIController
{
	GENERATED_BODY()

public:
	AAIControllerFootball();

	// Perception
	void UpdatePerception(float DeltaTime);
	bool CanSeeBall() const;
	TArray<AActor*> GetNearbyOpponents(float Radius) const;
	
	// Decision making
	void EvaluateUtilityScores();
	EAIBehavior SelectBestBehavior();
	
	// Behavior execution
	void ExecuteBehavior(EAIBehavior Behavior, float DeltaTime);
	
	// Debug visualization
	void DrawDebugInfo();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* PerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UBehaviorTreeComponent* BehaviorTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISteeringComponent* SteeringComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PerceptionRadius;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	FVector LastKnownBallLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	EAIBehavior CurrentBehavior;

	// Update throttling for performance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Performance")
	float PerceptionUpdateInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Performance")
	float DecisionUpdateInterval;

private:
	float TimeSinceLastPerceptionUpdate;
	float TimeSinceLastDecisionUpdate;
	
	// Stagger updates across AI agents
	float UpdateTimeOffset;
};
