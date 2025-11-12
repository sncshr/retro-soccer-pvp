// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DebugVisualizer.generated.h"

/**
 * 3D debug visualization utilities
 * Provides debug drawing for AI, networking, and gameplay systems
 */
UCLASS()
class POCKETSTRIKER_API UDebugVisualizer : public UObject
{
	GENERATED_BODY()

public:
	UDebugVisualizer();

	// 3D debug drawing
	void DrawAITargetLine(const FVector& From, const FVector& To);
	void DrawPerceptionRadius(const FVector& Center, float Radius);
	void DrawNavMesh();
	void DrawHitboxes(AActor* Actor);
	
	// Path visualization
	void DrawPredictedPath(const TArray<FVector>& Path, FColor Color);
	void DrawReconciledPath(const TArray<FVector>& Path, FColor Color);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bEnableVisualization;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float DebugLineDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float DebugLineThickness;

	// Toggle system
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ToggleVisualization() { bEnableVisualization = !bEnableVisualization; }

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetVisualizationEnabled(bool bEnabled) { bEnableVisualization = bEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Debug")
	bool IsVisualizationEnabled() const { return bEnableVisualization; }
};
