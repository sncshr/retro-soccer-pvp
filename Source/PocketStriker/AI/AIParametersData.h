// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AIParametersData.generated.h"

/**
 * Data asset for AI tuning parameters
 * Allows designers to adjust AI behavior without code changes
 */
UCLASS(BlueprintType)
class POCKETSTRIKER_API UAIParametersData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception", meta = (ToolTip = "Radius for AI perception in cm"))
	float PerceptionRadius = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception", meta = (ToolTip = "Update interval for perception in seconds"))
	float UpdateInterval = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (ToolTip = "Aggression level (0-1)"))
	float Aggression = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (ToolTip = "Tactical awareness level (0-1)"))
	float TacticalAwareness = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ToolTip = "Maximum AI movement speed in cm/s"))
	float MaxSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ToolTip = "Avoidance radius for collision in cm"))
	float AvoidanceRadius = 100.0f;
};
