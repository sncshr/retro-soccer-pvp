// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetworkTypes.h"
#include "NetworkGameState.generated.h"

class APocketStrikerPlayerController;

/**
 * Authoritative server game state manager
 * Handles server-side simulation and state broadcasting
 */
UCLASS()
class POCKETSTRIKER_API ANetworkGameState : public AActor
{
	GENERATED_BODY()
	
public:	
	ANetworkGameState();

	// Server-side input processing
	void ProcessClientInput(APocketStrikerPlayerController* Controller, const FInputPacket& Input);
	
	// State broadcasting
	void BroadcastStateUpdates();
	
	// Input validation (anti-cheat)
	bool ValidateInput(const FInputPacket& Input) const;
	
	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	float StateUpdateRate = 60.0f; // Updates per second

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	bool bEnableInputValidation = true;

	// Debug info
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
	int32 TotalInputsProcessed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
	int32 InvalidInputsRejected = 0;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	// Timing for fixed tick rate
	float TimeSinceLastUpdate;
	float UpdateInterval;

	// Track acknowledged sequences per client
	TMap<APocketStrikerPlayerController*, uint32> ClientAcknowledgedSequences;

	// Helper to create state update packet for a player
	FStateUpdatePacket CreateStateUpdate(APocketStrikerPlayerController* Controller, uint32 AckedSequence) const;
};
