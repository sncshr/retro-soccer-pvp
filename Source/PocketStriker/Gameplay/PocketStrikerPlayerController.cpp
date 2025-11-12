// Copyright Epic Games, Inc. All Rights Reserved.

#include "PocketStrikerPlayerController.h"
#include "GameplayTypes.h"
#include "PlayerTuningData.h"
#include "PlayerStateMachine.h"
#include "PlayerMovementComponent.h"
#include "ActionSystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "Tools/PerformanceProfiler.h"

APocketStrikerPlayerController::APocketStrikerPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create action system
	ActionSystem = CreateDefaultSubobject<UActionSystem>(TEXT("ActionSystem"));
}

void APocketStrikerPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Initialize Enhanced Input
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// Input mapping context will be set up in Blueprint or via data asset
	}

	// Find performance profiler if available
	if (APawn* ControlledPawn = GetPawn())
	{
		PerformanceProfiler = ControlledPawn->FindComponentByClass<UPerformanceProfiler>();
	}

	// Apply tuning data on begin play
	ApplyTuningData();

#if WITH_EDITOR
	// Set up hot-reload support in editor
	if (TuningData)
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &APocketStrikerPlayerController::OnDataAssetPropertyChanged);
	}
#endif
}

void APocketStrikerPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	ProcessInput(DeltaTime);
	
	// Record display timestamp for latency measurement (end of frame)
	if (PerformanceProfiler)
	{
		PerformanceProfiler->RecordDisplayTimestamp(FPlatformTime::Seconds());
	}
}

void APocketStrikerPlayerController::ProcessInput(float DeltaTime)
{
	// This will be called every frame to process and buffer input
	// Input buffering for network prediction will be implemented here
}

void APocketStrikerPlayerController::BufferInputCommand(const FInputCommand& Command)
{
	// Store input command in buffer for network prediction
	InputBuffer.Add(Command);
	
	// Keep buffer size manageable (last 60 frames = 1 second at 60fps)
	const int32 MaxBufferSize = 60;
	if (InputBuffer.Num() > MaxBufferSize)
	{
		InputBuffer.RemoveAt(0, InputBuffer.Num() - MaxBufferSize);
	}
}

TArray<FInputCommand> APocketStrikerPlayerController::GetUnacknowledgedInputs() const
{
	TArray<FInputCommand> UnacknowledgedInputs;
	
	for (const FInputCommand& Input : InputBuffer)
	{
		if (Input.SequenceNumber > LastAcknowledgedSequence)
		{
			UnacknowledgedInputs.Add(Input);
		}
	}
	
	return UnacknowledgedInputs;
}

void APocketStrikerPlayerController::AcknowledgeInput(uint32 SequenceNumber)
{
	LastAcknowledgedSequence = SequenceNumber;
	
	// Remove old acknowledged inputs from buffer
	InputBuffer.RemoveAll([SequenceNumber](const FInputCommand& Input)
	{
		return Input.SequenceNumber <= SequenceNumber;
	});
}

void APocketStrikerPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Set up Enhanced Input bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Movement bindings will be set up here when Input Actions are created
		// For now, we'll use legacy input as a fallback
	}
}

void APocketStrikerPlayerController::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	
	// Record input timestamp for latency measurement
	if (PerformanceProfiler)
	{
		PerformanceProfiler->RecordInputTimestamp(FPlatformTime::Seconds());
	}
	
	if (APawn* ControlledPawn = GetPawn())
	{
		// Get forward and right vectors
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		// Add movement input
		ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);
		
		// Buffer input for network prediction
		FInputCommand Command;
		Command.SequenceNumber = ++CurrentInputSequence;
		Command.ClientTimestamp = GetWorld()->GetTimeSeconds();
		Command.MovementInput = MovementVector;
		
		BufferInputCommand(Command);
	}
}

void APocketStrikerPlayerController::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	// Add yaw and pitch input
	AddYawInput(LookAxisVector.X);
	AddPitchInput(LookAxisVector.Y);
}

void APocketStrikerPlayerController::Sprint()
{
	bIsSprintPressed = true;
	
	// Buffer sprint action
	FInputCommand Command;
	Command.SequenceNumber = ++CurrentInputSequence;
	Command.ClientTimestamp = GetWorld()->GetTimeSeconds();
	Command.ActionFlags |= FInputCommand::FLAG_SPRINT;
	
	BufferInputCommand(Command);
}

void APocketStrikerPlayerController::StopSprint()
{
	bIsSprintPressed = false;
}

void APocketStrikerPlayerController::Tackle()
{
	// Buffer tackle action
	FInputCommand Command;
	Command.SequenceNumber = ++CurrentInputSequence;
	Command.ClientTimestamp = GetWorld()->GetTimeSeconds();
	Command.ActionFlags |= FInputCommand::FLAG_TACKLE;
	
	BufferInputCommand(Command);
	
	// Execute tackle action through state machine
	if (ACharacter* Character = GetCharacter())
	{
		// State machine integration will be added when wiring systems together
	}
}

void APocketStrikerPlayerController::Kick()
{
	// Buffer kick action
	FInputCommand Command;
	Command.SequenceNumber = ++CurrentInputSequence;
	Command.ClientTimestamp = GetWorld()->GetTimeSeconds();
	Command.ActionFlags |= FInputCommand::FLAG_KICK;
	
	BufferInputCommand(Command);
	
	// Execute kick action through state machine
	if (ACharacter* Character = GetCharacter())
	{
		// State machine integration will be added when wiring systems together
	}
}

void APocketStrikerPlayerController::Pass()
{
	// Buffer pass action
	FInputCommand Command;
	Command.SequenceNumber = ++CurrentInputSequence;
	Command.ClientTimestamp = GetWorld()->GetTimeSeconds();
	Command.ActionFlags |= FInputCommand::FLAG_PASS;
	
	BufferInputCommand(Command);
	
	// Execute pass action through state machine
	if (ACharacter* Character = GetCharacter())
	{
		// State machine integration will be added when wiring systems together
	}
}

void APocketStrikerPlayerController::ApplyTuningData()
{
	if (!TuningData)
	{
		UE_LOG(LogTemp, Warning, TEXT("APocketStrikerPlayerController::ApplyTuningData - No TuningData assigned!"));
		return;
	}

	// Apply tuning data to movement component
	if (ACharacter* Character = GetCharacter())
	{
		if (UPlayerMovementComponent* MovementComp = Cast<UPlayerMovementComponent>(Character->GetCharacterMovement()))
		{
			MovementComp->ApplyTuningData(TuningData);
			UE_LOG(LogTemp, Log, TEXT("Applied PlayerTuningData to movement component"));
		}
	}

	// Apply tuning data to action system
	if (ActionSystem)
	{
		ActionSystem->ApplyTuningData(TuningData);
	}
}

void APocketStrikerPlayerController::OnTuningDataChanged()
{
	UE_LOG(LogTemp, Log, TEXT("TuningData changed - reapplying parameters"));
	ApplyTuningData();
}

#if WITH_EDITOR
void APocketStrikerPlayerController::OnDataAssetPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	// Check if the changed object is our TuningData
	if (Object == TuningData)
	{
		OnTuningDataChanged();
	}
}
#endif

// Network RPC implementations
void APocketStrikerPlayerController::ServerSendInput_Implementation(const FInputCommand& Input)
{
	// Server receives input from client
	// Process input on server using authoritative simulation
	
	if (ACharacter* Character = GetCharacter())
	{
		if (UPlayerMovementComponent* MovementComp = Cast<UPlayerMovementComponent>(Character->GetCharacterMovement()))
		{
			// Simulate movement on server
			float DeltaTime = 1.0f / 60.0f; // Fixed timestep
			MovementComp->SimulateMovement(Input, DeltaTime);
		}
	}

	// Send state update back to client
	if (ACharacter* Character = GetCharacter())
	{
		FVector Position = Character->GetActorLocation();
		FVector Velocity = FVector::ZeroVector;
		float Stamina = 100.0f;
		uint8 State = 0;

		if (UPlayerMovementComponent* MovementComp = Cast<UPlayerMovementComponent>(Character->GetCharacterMovement()))
		{
			Velocity = MovementComp->Velocity;
			Stamina = MovementComp->CurrentStamina;
		}

		if (UPlayerStateMachine* StateMachine = Character->FindComponentByClass<UPlayerStateMachine>())
		{
			State = static_cast<uint8>(StateMachine->CurrentState);
		}

		ClientReceiveStateUpdate(Position, Velocity, Stamina, State, Input.SequenceNumber);
	}
}

bool APocketStrikerPlayerController::ServerSendInput_Validate(const FInputCommand& Input)
{
	// Validate input on server to prevent cheating
	
	// Check input ranges
	if (FMath::Abs(Input.MovementInput.X) > 1.0f || FMath::Abs(Input.MovementInput.Y) > 1.0f)
	{
		return false;
	}

	if (FMath::Abs(Input.LookInput.X) > 1.0f || FMath::Abs(Input.LookInput.Y) > 1.0f)
	{
		return false;
	}

	// Validate action flags
	const uint32 ValidFlags = FInputCommand::FLAG_SPRINT | FInputCommand::FLAG_TACKLE | 
		FInputCommand::FLAG_KICK | FInputCommand::FLAG_PASS;
	if ((Input.ActionFlags & ~ValidFlags) != 0)
	{
		return false;
	}

	return true;
}

void APocketStrikerPlayerController::ClientReceiveStateUpdate_Implementation(const FVector& Position, const FVector& Velocity, float Stamina, uint8 State, uint32 AckedSequence)
{
	// Client receives authoritative state from server
	// This triggers reconciliation if needed
	
	AcknowledgeInput(AckedSequence);

	// Check if we need reconciliation
	if (ACharacter* Character = GetCharacter())
	{
		FVector CurrentPosition = Character->GetActorLocation();
		float PositionError = FVector::Dist(CurrentPosition, Position);

		// If error is significant, reconcile
		const float CorrectionThreshold = 10.0f; // 10cm
		if (PositionError > CorrectionThreshold)
		{
			// Apply server correction
			Character->SetActorLocation(Position);

			if (UPlayerMovementComponent* MovementComp = Cast<UPlayerMovementComponent>(Character->GetCharacterMovement()))
			{
				MovementComp->Velocity = Velocity;
				MovementComp->CurrentStamina = Stamina;
			}

			if (UPlayerStateMachine* StateMachine = Character->FindComponentByClass<UPlayerStateMachine>())
			{
				StateMachine->CurrentState = static_cast<EPlayerState>(State);
			}

			// Replay unacknowledged inputs
			TArray<FInputCommand> UnackedInputs = GetUnacknowledgedInputs();
			for (const FInputCommand& UnackedInput : UnackedInputs)
			{
				if (UPlayerMovementComponent* MovementComp = Cast<UPlayerMovementComponent>(Character->GetCharacterMovement()))
				{
					float DeltaTime = 1.0f / 60.0f;
					MovementComp->SimulateMovement(UnackedInput, DeltaTime);
				}
			}
		}
	}
}
