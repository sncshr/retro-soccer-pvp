// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NetworkTypes.h"
#include "NetworkReconciler.generated.h"

class UNetworkPrediction;
class UPlayerMovementComponent;
class UPlayerStateMachine;

/**
 * Server reconciliation component
 * Handles correction and state replay for client-side prediction
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POCKETSTRIKER_API UNetworkReconciler : public UActorComponent
{
	GENERATED_BODY()

public:
	UNetworkReconciler();

	// Correction handling
	void OnServerCorrection(const FStateUpdatePacket& Correction);
	bool NeedsReconciliation(const FStateUpdatePacket& ServerState) const;
	
	// State replay
	void ReplayInputs(const FVector& CorrectedPosition, uint32 FromSequence);
	
	// Smoothing
	void SmoothCorrection(const FVector& TargetPosition, float DeltaTime);

	// Apply network parameters
	void ApplyNetworkParams(float InCorrectionThreshold, float InSmoothingSpeed);

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (ToolTip = "Distance threshold before correction in cm"))
	float CorrectionThreshold = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (ToolTip = "Speed of correction smoothing"))
	float SmoothingSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (ToolTip = "Enable visual smoothing for small corrections"))
	bool bEnableSmoothing = true;

	// Debug info
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
	FVector LastCorrectionDelta;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
	float LastCorrectionTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
	int32 TotalCorrections = 0;

	// Path visualization
	UFUNCTION(BlueprintCallable, Category = "Debug")
	TArray<FVector> GetReconciledPath() const { return ReconciledPath; }

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DrawDebugPaths();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Cached component references
	UPROPERTY()
	UNetworkPrediction* PredictionComponent;

	UPROPERTY()
	UPlayerMovementComponent* MovementComponent;

	UPROPERTY()
	UPlayerStateMachine* StateMachine;

	// Smoothing state
	FVector SmoothingTarget;
	bool bIsSmoothing = false;

	// Path history for visualization
	TArray<FVector> ReconciledPath;
	static constexpr int32 MaxPathPoints = 100;
};
