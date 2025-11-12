// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerStateMachine.h"

UPlayerStateMachine::UPlayerStateMachine()
	: CurrentState(EPlayerState::Idle)
	, StateTimer(0.0f)
{
}

bool UPlayerStateMachine::CanTransitionTo(EPlayerState NewState) const
{
	// Define valid state transitions
	switch (CurrentState)
	{
		case EPlayerState::Idle:
			// Can transition to any state from idle
			return true;

		case EPlayerState::Move:
			// Can transition to sprint, tackle, kick, pass, or back to idle
			return NewState == EPlayerState::Idle ||
				   NewState == EPlayerState::Sprint ||
				   NewState == EPlayerState::Tackle ||
				   NewState == EPlayerState::Kick ||
				   NewState == EPlayerState::Pass;

		case EPlayerState::Sprint:
			// Can transition to move, tackle, kick, or idle
			return NewState == EPlayerState::Idle ||
				   NewState == EPlayerState::Move ||
				   NewState == EPlayerState::Tackle ||
				   NewState == EPlayerState::Kick;

		case EPlayerState::Tackle:
			// Tackle is a committed action, can only return to idle when complete
			return NewState == EPlayerState::Idle;

		case EPlayerState::Kick:
			// Kick is a committed action, can only return to idle when complete
			return NewState == EPlayerState::Idle;

		case EPlayerState::Pass:
			// Pass is a committed action, can only return to idle when complete
			return NewState == EPlayerState::Idle;

		default:
			return false;
	}
}

void UPlayerStateMachine::EnterState(EPlayerState State)
{
	if (!CanTransitionTo(State))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid state transition from %d to %d"), 
			static_cast<int32>(CurrentState), static_cast<int32>(State));
		return;
	}

	// Exit current state
	ExitState(CurrentState);

	// Update state
	EPlayerState PreviousState = CurrentState;
	CurrentState = State;
	StateTimer = 0.0f;

	// Perform state-specific enter logic
	switch (State)
	{
		case EPlayerState::Idle:
			UE_LOG(LogTemp, Verbose, TEXT("Entered Idle state"));
			break;

		case EPlayerState::Move:
			UE_LOG(LogTemp, Verbose, TEXT("Entered Move state"));
			break;

		case EPlayerState::Sprint:
			UE_LOG(LogTemp, Verbose, TEXT("Entered Sprint state"));
			break;

		case EPlayerState::Tackle:
			UE_LOG(LogTemp, Verbose, TEXT("Entered Tackle state"));
			break;

		case EPlayerState::Kick:
			UE_LOG(LogTemp, Verbose, TEXT("Entered Kick state"));
			break;

		case EPlayerState::Pass:
			UE_LOG(LogTemp, Verbose, TEXT("Entered Pass state"));
			break;
	}
}

void UPlayerStateMachine::ExitState(EPlayerState State)
{
	// Perform state-specific exit logic
	switch (State)
	{
		case EPlayerState::Idle:
			// No cleanup needed for idle
			break;

		case EPlayerState::Move:
			// No cleanup needed for move
			break;

		case EPlayerState::Sprint:
			// Sprint exit handled by movement component
			break;

		case EPlayerState::Tackle:
			// Tackle cleanup
			UE_LOG(LogTemp, Verbose, TEXT("Exited Tackle state"));
			break;

		case EPlayerState::Kick:
			// Kick cleanup
			UE_LOG(LogTemp, Verbose, TEXT("Exited Kick state"));
			break;

		case EPlayerState::Pass:
			// Pass cleanup
			UE_LOG(LogTemp, Verbose, TEXT("Exited Pass state"));
			break;
	}
}

void UPlayerStateMachine::UpdateState(float DeltaTime)
{
	StateTimer += DeltaTime;

	// Update current state logic
	switch (CurrentState)
	{
		case EPlayerState::Idle:
			// Idle state has no time-based logic
			break;

		case EPlayerState::Move:
			// Move state has no time-based logic
			break;

		case EPlayerState::Sprint:
			// Sprint state managed by movement component
			break;

		case EPlayerState::Tackle:
			// Tackle has a fixed duration (will be configured via action system)
			// For now, use a default duration
			if (StateTimer >= 0.5f)
			{
				EnterState(EPlayerState::Idle);
			}
			break;

		case EPlayerState::Kick:
			// Kick has a fixed duration
			if (StateTimer >= 0.4f)
			{
				EnterState(EPlayerState::Idle);
			}
			break;

		case EPlayerState::Pass:
			// Pass has a fixed duration
			if (StateTimer >= 0.3f)
			{
				EnterState(EPlayerState::Idle);
			}
			break;
	}
}

bool UPlayerStateMachine::CanPerformAction(EPlayerAction Action) const
{
	// Check if the action can be performed in the current state
	switch (Action)
	{
		case EPlayerAction::None:
			return true;

		case EPlayerAction::Sprint:
			// Can sprint from idle or move states
			return CurrentState == EPlayerState::Idle || 
				   CurrentState == EPlayerState::Move;

		case EPlayerAction::Tackle:
			// Can tackle from idle, move, or sprint states
			return CurrentState == EPlayerState::Idle || 
				   CurrentState == EPlayerState::Move || 
				   CurrentState == EPlayerState::Sprint;

		case EPlayerAction::Kick:
			// Can kick from idle, move, or sprint states
			return CurrentState == EPlayerState::Idle || 
				   CurrentState == EPlayerState::Move || 
				   CurrentState == EPlayerState::Sprint;

		case EPlayerAction::Pass:
			// Can pass from idle, move, or sprint states
			return CurrentState == EPlayerState::Idle || 
				   CurrentState == EPlayerState::Move || 
				   CurrentState == EPlayerState::Sprint;

		default:
			return false;
	}
}

void UPlayerStateMachine::ExecuteAction(EPlayerAction Action)
{
	if (!CanPerformAction(Action))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot perform action %d in state %d"), 
			static_cast<int32>(Action), static_cast<int32>(CurrentState));
		return;
	}

	// Map actions to states
	switch (Action)
	{
		case EPlayerAction::None:
			// No action
			break;

		case EPlayerAction::Sprint:
			EnterState(EPlayerState::Sprint);
			break;

		case EPlayerAction::Tackle:
			EnterState(EPlayerState::Tackle);
			break;

		case EPlayerAction::Kick:
			EnterState(EPlayerState::Kick);
			break;

		case EPlayerAction::Pass:
			EnterState(EPlayerState::Pass);
			break;
	}
}
