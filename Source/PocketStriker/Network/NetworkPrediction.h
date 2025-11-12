// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NetworkTypes.h"
#include "NetworkPrediction.generated.h"

class UPlayerMovementComponent;
class UPlayerStateMachine;

/**
 * Client-side prediction component
 * Handles input buffering and local simulation for responsive networked gameplay
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POCKETSTRIKER_API UNetworkPrediction : public UActorComponent
{
	GENERATED_BODY()

public:
	UNetworkPrediction();

	// Input buffering
	void BufferInput(const FInputPacket& Input);
	TArray<FInputPacket> GetUnacknowledgedInputs(uint32 LastAckedSequence) const;
	void ClearAcknowledgedInputs(uint32 AckedSequence);
	
	// Local simulation
	void PredictMovement(float DeltaTime);
	void SimulateInput(const FInputPacket& Input, float DeltaTime);
	
	// State tracking
	void SavePredictionState(uint32 SequenceNumber);
	FPredictionState GetStateAtSequence(uint32 SequenceNumber) const;
	void ClearOldStates(uint32 OldestNeededSequence);

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	int32 MaxInputBufferSize = 128;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	int32 MaxStateHistorySize = 128;

	// Path visualization
	UFUNCTION(BlueprintCallable, Category = "Debug")
	TArray<FVector> GetPredictedPath() const { return PredictedPath; }

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DrawDebugPaths();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Input buffer for unacknowledged inputs
	UPROPERTY()
	TArray<FInputPacket> InputBuffer;

	// State history for reconciliation
	UPROPERTY()
	TArray<FPredictionState> StateHistory;

	// Current sequence number
	uint32 CurrentSequence;

	// Cached component references
	UPROPERTY()
	UPlayerMovementComponent* MovementComponent;

	UPROPERTY()
	UPlayerStateMachine* StateMachine;

	// Helper to capture current state
	FPredictionState CaptureCurrentState(uint32 SequenceNumber) const;

	// Path history for visualization (mutable for const methods)
	mutable TArray<FVector> PredictedPath;
	static constexpr int32 MaxPathPoints = 100;
};
