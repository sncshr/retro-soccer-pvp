// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlayerStateMachine.generated.h"

UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	Idle,
	Move,
	Sprint,
	Tackle,
	Kick,
	Pass
};

UENUM(BlueprintType)
enum class EPlayerAction : uint8
{
	None,
	Sprint,
	Tackle,
	Kick,
	Pass
};

/**
 * State machine for player action management
 * Handles state transitions and action validation
 */
UCLASS()
class POCKETSTRIKER_API UPlayerStateMachine : public UObject
{
	GENERATED_BODY()

public:
	UPlayerStateMachine();

	// State transitions
	bool CanTransitionTo(EPlayerState NewState) const;
	void EnterState(EPlayerState State);
	void ExitState(EPlayerState State);
	void UpdateState(float DeltaTime);
	
	// Action validation
	bool CanPerformAction(EPlayerAction Action) const;
	void ExecuteAction(EPlayerAction Action);
	
	// Current state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EPlayerState CurrentState;

protected:
	float StateTimer;
};
