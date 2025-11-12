// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AIControllerFootball.h"
#include "FootballAIUtility.generated.h"

class AAIControllerFootball;

USTRUCT(BlueprintType)
struct FAIContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BallLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> OpponentPositions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector AIPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceToBall;
};

USTRUCT(BlueprintType)
struct FUtilityScore
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAIBehavior Behavior;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Score;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Reasoning;
};

/**
 * Utility-based AI decision system
 * Scores behaviors based on tactical context
 */
UCLASS()
class POCKETSTRIKER_API UFootballAIUtility : public UObject
{
	GENERATED_BODY()

public:
	UFootballAIUtility();

	// Utility calculation
	float ScorePress(const FAIContext& Context);
	float ScoreSupport(const FAIContext& Context);
	float ScoreIntercept(const FAIContext& Context);
	float ScoreCover(const FAIContext& Context);
	float ScoreRetreat(const FAIContext& Context);
	
	// Context evaluation
	FAIContext BuildContext(AAIControllerFootball* Controller);
};
