// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkGameState.h"
#include "../Gameplay/PocketStrikerPlayerController.h"
#include "../Gameplay/PlayerMovementComponent.h"
#include "../Gameplay/PlayerStateMachine.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

ANetworkGameState::ANetworkGameState()
{
	PrimaryActorTick.bCanEverTick = true;
	TimeSinceLastUpdate = 0.0f;
	UpdateInterval = 1.0f / StateUpdateRate;
}

void ANetworkGameState::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateInterval = 1.0f / StateUpdateRate;
}

void ANetworkGameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Only run on server
	if (!HasAuthority())
	{
		return;
	}

	// Accumulate time
	TimeSinceLastUpdate += DeltaTime;

	// Broadcast state updates at fixed rate
	if (TimeSinceLastUpdate >= UpdateInterval)
	{
		BroadcastStateUpdates();
		TimeSinceLastUpdate = 0.0f;
	}
}

bool ANetworkGameState::ValidateInput(const FInputPacket& Input) const
{
	if (!bEnableInputValidation)
	{
		return true;
	}

	// Validate packet integrity
	if (!Input.IsValid())
	{
		return false;
	}

	// Validate input ranges
	if (FMath::Abs(Input.MovementInput.X) > 1.0f || FMath::Abs(Input.MovementInput.Y) > 1.0f)
	{
		return false;
	}

	if (FMath::Abs(Input.LookInput.X) > 1.0f || FMath::Abs(Input.LookInput.Y) > 1.0f)
	{
		return false;
	}

	// Validate action flags (only valid bits should be set)
	const uint32 ValidFlags = FInputPacket::FLAG_SPRINT | FInputPacket::FLAG_TACKLE | 
		FInputPacket::FLAG_KICK | FInputPacket::FLAG_PASS;
	if ((Input.ActionFlags & ~ValidFlags) != 0)
	{
		return false;
	}

	return true;
}

void ANetworkGameState::ProcessClientInput(APocketStrikerPlayerController* Controller, const FInputPacket& Input)
{
	if (!Controller || !HasAuthority())
	{
		return;
	}

	// Validate input
	if (!ValidateInput(Input))
	{
		InvalidInputsRejected++;
		return;
	}

	TotalInputsProcessed++;

	// Get the controlled character
	ACharacter* Character = Controller->GetCharacter();
	if (!Character)
	{
		return;
	}

	// Get movement component
	UPlayerMovementComponent* MovementComp = Character->FindComponentByClass<UPlayerMovementComponent>();
	if (!MovementComp)
	{
		return;
	}

	// Convert to FInputCommand for simulation
	FInputCommand Command;
	Command.SequenceNumber = Input.SequenceNumber;
	Command.ClientTimestamp = Input.ClientTimestamp;
	Command.MovementInput = Input.MovementInput;
	Command.LookInput = Input.LookInput;
	Command.ActionFlags = Input.ActionFlags;

	// Simulate movement on server using same logic as client
	float DeltaTime = UpdateInterval; // Use fixed timestep
	MovementComp->SimulateMovement(Command, DeltaTime);

	// Update acknowledged sequence for this client
	ClientAcknowledgedSequences.Add(Controller, Input.SequenceNumber);
}

void ANetworkGameState::BroadcastStateUpdates()
{
	if (!HasAuthority())
	{
		return;
	}

	// Get all player controllers
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Iterate through all player controllers
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APocketStrikerPlayerController* PC = Cast<APocketStrikerPlayerController>(It->Get());
		if (!PC)
		{
			continue;
		}

		// Get acknowledged sequence for this client
		uint32* AckedSeq = ClientAcknowledgedSequences.Find(PC);
		uint32 AcknowledgedSequence = AckedSeq ? *AckedSeq : 0;

		// Create state update packet
		FStateUpdatePacket StateUpdate = CreateStateUpdate(PC, AcknowledgedSequence);

		// Send to client (in a real implementation, this would use RPC or replication)
		// For now, we'll call a method on the controller directly
		// TODO: Implement proper network replication or RPC
		
		// This is a placeholder - in production, you'd use:
		// PC->ClientReceiveStateUpdate(StateUpdate);
	}
}

FStateUpdatePacket ANetworkGameState::CreateStateUpdate(APocketStrikerPlayerController* Controller, uint32 AckedSequence) const
{
	FStateUpdatePacket Packet;
	Packet.AcknowledgedSequence = AckedSequence;
	
	if (UWorld* World = GetWorld())
	{
		Packet.ServerTimestamp = World->GetTimeSeconds();
	}

	// Get character state
	ACharacter* Character = Controller->GetCharacter();
	if (Character)
	{
		Packet.AuthoritativePosition = Character->GetActorLocation();

		UPlayerMovementComponent* MovementComp = Character->FindComponentByClass<UPlayerMovementComponent>();
		if (MovementComp)
		{
			Packet.AuthoritativeVelocity = MovementComp->Velocity;
			Packet.AuthoritativeStamina = MovementComp->CurrentStamina;
		}

		UPlayerStateMachine* StateMachine = Character->FindComponentByClass<UPlayerStateMachine>();
		if (StateMachine)
		{
			Packet.AuthoritativeState = static_cast<uint8>(StateMachine->CurrentState);
		}
	}

	// Calculate checksum
	Packet.Checksum = Packet.CalculateChecksum();

	return Packet;
}
