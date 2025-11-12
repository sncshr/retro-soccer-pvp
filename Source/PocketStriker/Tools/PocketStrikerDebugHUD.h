// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PocketStrikerDebugHUD.generated.h"

/**
 * Debug HUD for performance and network metrics
 * Displays FPS, frame timings, network stats, and AI debug info
 */
UCLASS()
class POCKETSTRIKER_API APocketStrikerDebugHUD : public AHUD
{
	GENERATED_BODY()

public:
	APocketStrikerDebugHUD();

	// Performance metrics
	void DrawFPSCounter(UCanvas* Canvas);
	void DrawFrameTimings(UCanvas* Canvas);
	
	// Network metrics
	void DrawNetworkStats(UCanvas* Canvas);
	void DrawLastCorrection(UCanvas* Canvas);
	
	// AI debug
	void DrawAIStates(UCanvas* Canvas);
	void DrawAIPerception(UCanvas* Canvas);
	
	// Motion matching debug
	void DrawMotionMatchingInfo(UCanvas* Canvas);
	void DrawCandidateScores(UCanvas* Canvas);
	
	// Input visualization
	void DrawInputBuffer(UCanvas* Canvas);
	void DrawPredictedPath(UCanvas* Canvas);
	void DrawReconciledPath(UCanvas* Canvas);
	
	// Collision visualization
	void DrawHitboxes(UCanvas* Canvas);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowNetworkPaths = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowHitboxes = true;

protected:
	virtual void DrawHUD() override;
	virtual void BeginPlay() override;
};
