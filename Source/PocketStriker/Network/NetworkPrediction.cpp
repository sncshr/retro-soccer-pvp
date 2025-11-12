// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkPrediction.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "../Gameplay/PlayerMovementComponent.h"
#include "../Gameplay/PlayerStateMachine.h"
#include "../Gameplay/PocketStrikerPlayerController.h"

UNetworkPrediction::UNetworkPrediction()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentSequence = 0;
}

void UNetworkPrediction::BeginPlay()
{
	Super::BeginPlay();

	// Cache component references
	AActor* Owner = GetOwner();
	if (Owner)
	{
		MovementComponent = Owner->FindComponentByClass<UPlayerMovementComponent>();
		StateMachine = Owner->FindComponentByClass<UPlayerStateMachine>();
	}

	// Reserve space for buffers
	InputBuffer.Reserve(MaxInputBufferSize);
	StateHistory.Reserve(MaxStateHistorySize);
}

void UNetworkPrediction::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Prediction happens in response to input, not every tick
	// This tick is mainly for cleanup and monitoring
}

void UNetworkPrediction::BufferInput(const FInputPacket& Input)
{
	// Add input to buffer
	InputBuffer.Add(Input);

	// Trim buffer if it exceeds max size
	if (InputBuffer.Num() > MaxInputBufferSize)
	{
		// Remove oldest inputs
		int32 NumToRemove = InputBuffer.Num() - MaxInputBufferSize;
		InputBuffer.RemoveAt(0, NumToRemove);
	}
}

TArray<FInputPacket> UNetworkPrediction::GetUnacknowledgedInputs(uint32 LastAckedSequence) const
{
	TArray<FInputPacket> UnackedInputs;

	for (const FInputPacket& Input : InputBuffer)
	{
		if (Input.SequenceNumber > LastAckedSequence)
		{
			UnackedInputs.Add(Input);
		}
	}

	return UnackedInputs;
}

void UNetworkPrediction::ClearAcknowledgedInputs(uint32 AckedSequence)
{
	// Remove all inputs up to and including the acknowledged sequence
	InputBuffer.RemoveAll([AckedSequence](const FInputPacket& Input)
	{
		return Input.SequenceNumber <= AckedSequence;
	});
}

void UNetworkPrediction::PredictMovement(float DeltaTime)
{
	// This is called to run local prediction
	// The actual simulation happens in SimulateInput
	
	if (!MovementComponent)
	{
		return;
	}

	// Movement component handles the actual simulation
	// This method is here for future expansion if needed
}

void UNetworkPrediction::SimulateInput(const FInputPacket& Input, float DeltaTime)
{
	if (!MovementComponent || !StateMachine)
	{
		return;
	}

	// Convert FInputPacket to FInputCommand for movement component
	FInputCommand Command;
	Command.SequenceNumber = Input.SequenceNumber;
	Command.ClientTimestamp = Input.ClientTimestamp;
	Command.MovementInput = Input.MovementInput;
	Command.LookInput = Input.LookInput;
	Command.ActionFlags = Input.ActionFlags;

	// Simulate movement with this input
	MovementComponent->SimulateMovement(Command, DeltaTime);

	// Save the resulting state
	SavePredictionState(Input.SequenceNumber);
}

void UNetworkPrediction::SavePredictionState(uint32 SequenceNumber)
{
	FPredictionState State = CaptureCurrentState(SequenceNumber);
	
	// Add to history
	StateHistory.Add(State);

	// Trim history if it exceeds max size
	if (StateHistory.Num() > MaxStateHistorySize)
	{
		int32 NumToRemove = StateHistory.Num() - MaxStateHistorySize;
		StateHistory.RemoveAt(0, NumToRemove);
	}
}

FPredictionState UNetworkPrediction::GetStateAtSequence(uint32 SequenceNumber) const
{
	// Find state with matching sequence number
	for (const FPredictionState& State : StateHistory)
	{
		if (State.SequenceNumber == SequenceNumber)
		{
			return State;
		}
	}

	// Return empty state if not found
	return FPredictionState();
}

void UNetworkPrediction::ClearOldStates(uint32 OldestNeededSequence)
{
	// Remove states older than the oldest needed sequence
	StateHistory.RemoveAll([OldestNeededSequence](const FPredictionState& State)
	{
		return State.SequenceNumber < OldestNeededSequence;
	});
}

FPredictionState UNetworkPrediction::CaptureCurrentState(uint32 SequenceNumber) const
{
	FPredictionState State;
	State.SequenceNumber = SequenceNumber;
	State.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	AActor* Owner = GetOwner();
	if (Owner)
	{
		State.Position = Owner->GetActorLocation();
		
		if (MovementComponent)
		{
			State.Velocity = MovementComponent->Velocity;
			State.Stamina = MovementComponent->CurrentStamina;
		}

		if (StateMachine)
		{
			State.State = static_cast<uint8>(StateMachine->CurrentState);
		}
	}

	// Track predicted path
	PredictedPath.Add(State.Position);
	if (PredictedPath.Num() > MaxPathPoints)
	{
		PredictedPath.RemoveAt(0);
	}

	return State;
}

void UNetworkPrediction::DrawDebugPaths()
{
	UWorld* World = GetWorld();
	if (!World || PredictedPath.Num() < 2)
	{
		return;
	}

	// Draw predicted path in green
	for (int32 i = 0; i < PredictedPath.Num() - 1; ++i)
	{
		DrawDebugLine(World, PredictedPath[i], PredictedPath[i + 1], FColor::Green, false, 0.1f, 0, 3.0f);
	}

	// Draw current position marker
	AActor* Owner = GetOwner();
	if (Owner)
	{
		DrawDebugSphere(World, Owner->GetActorLocation(), 25.0f, 12, FColor::Green, false, 0.1f, 0, 2.0f);
	}

	// Draw input buffer visualization
	if (InputBuffer.Num() > 0)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			FVector BasePos = Owner->GetActorLocation() + FVector(0, 0, 150.0f);
			DrawDebugString(World, BasePos, FString::Printf(TEXT("Buffered: %d"), InputBuffer.Num()), 
				nullptr, FColor::Cyan, 0.1f, true, 1.0f);
		}
	}
}
