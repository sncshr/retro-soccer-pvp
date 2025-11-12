// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkReconciler.h"
#include "NetworkPrediction.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "../Gameplay/PlayerMovementComponent.h"
#include "../Gameplay/PlayerStateMachine.h"
#include "../Gameplay/PocketStrikerPlayerController.h"
#include "../Tools/PerformanceProfiler.h"

UNetworkReconciler::UNetworkReconciler()
{
	PrimaryComponentTick.bCanEverTick = true;
	LastCorrectionDelta = FVector::ZeroVector;
	LastCorrectionTime = 0.0f;
}

void UNetworkReconciler::BeginPlay()
{
	Super::BeginPlay();

	// Cache component references
	AActor* Owner = GetOwner();
	if (Owner)
	{
		PredictionComponent = Owner->FindComponentByClass<UNetworkPrediction>();
		MovementComponent = Owner->FindComponentByClass<UPlayerMovementComponent>();
		StateMachine = Owner->FindComponentByClass<UPlayerStateMachine>();
	}
}

void UNetworkReconciler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Handle smoothing if active
	if (bIsSmoothing && bEnableSmoothing)
	{
		SmoothCorrection(SmoothingTarget, DeltaTime);
	}
}

void UNetworkReconciler::ApplyNetworkParams(float InCorrectionThreshold, float InSmoothingSpeed)
{
	CorrectionThreshold = InCorrectionThreshold;
	SmoothingSpeed = InSmoothingSpeed;
}

bool UNetworkReconciler::NeedsReconciliation(const FStateUpdatePacket& ServerState) const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	// Calculate position error
	FVector CurrentPosition = Owner->GetActorLocation();
	float PositionError = FVector::Dist(CurrentPosition, ServerState.AuthoritativePosition);

	// Check if error exceeds threshold
	return PositionError > CorrectionThreshold;
}

void UNetworkReconciler::OnServerCorrection(const FStateUpdatePacket& Correction)
{
	if (!PredictionComponent || !MovementComponent || !StateMachine)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Check if reconciliation is needed
	if (!NeedsReconciliation(Correction))
	{
		// Small error, just acknowledge and continue
		if (APocketStrikerPlayerController* PC = Cast<APocketStrikerPlayerController>(Owner->GetInstigatorController()))
		{
			PC->AcknowledgeInput(Correction.AcknowledgedSequence);
		}
		return;
	}

	// Calculate correction delta for debugging
	FVector CurrentPosition = Owner->GetActorLocation();
	LastCorrectionDelta = Correction.AuthoritativePosition - CurrentPosition;
	LastCorrectionTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	TotalCorrections++;

	// Record correction for performance profiling
	float CorrectionMagnitude = LastCorrectionDelta.Size();
	if (UPerformanceProfiler* Profiler = Owner->FindComponentByClass<UPerformanceProfiler>())
	{
		Profiler->RecordNetworkCorrection(CorrectionMagnitude);
	}

	// Apply server state
	Owner->SetActorLocation(Correction.AuthoritativePosition);
	MovementComponent->Velocity = Correction.AuthoritativeVelocity;
	MovementComponent->CurrentStamina = Correction.AuthoritativeStamina;
	StateMachine->CurrentState = static_cast<EPlayerState>(Correction.AuthoritativeState);

	// Replay unacknowledged inputs
	ReplayInputs(Correction.AuthoritativePosition, Correction.AcknowledgedSequence);

	// Acknowledge the input
	if (APocketStrikerPlayerController* PC = Cast<APocketStrikerPlayerController>(Owner->GetInstigatorController()))
	{
		PC->AcknowledgeInput(Correction.AcknowledgedSequence);
	}

	// Clean up old prediction states
	PredictionComponent->ClearOldStates(Correction.AcknowledgedSequence);
}

void UNetworkReconciler::ReplayInputs(const FVector& CorrectedPosition, uint32 FromSequence)
{
	if (!PredictionComponent || !MovementComponent)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Get controller to access unacknowledged inputs
	APocketStrikerPlayerController* PC = Cast<APocketStrikerPlayerController>(Owner->GetInstigatorController());
	if (!PC)
	{
		return;
	}

	// Get all inputs after the acknowledged sequence
	TArray<FInputPacket> UnackedInputs = PredictionComponent->GetUnacknowledgedInputs(FromSequence);

	// Replay each input
	for (const FInputPacket& Input : UnackedInputs)
	{
		// Simulate this input
		// Note: DeltaTime should ideally be stored with the input, using fixed timestep for now
		float DeltaTime = 1.0f / 60.0f; // Assume 60 FPS
		PredictionComponent->SimulateInput(Input, DeltaTime);
	}
}

void UNetworkReconciler::SmoothCorrection(const FVector& TargetPosition, float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		bIsSmoothing = false;
		return;
	}

	FVector CurrentPosition = Owner->GetActorLocation();
	float Distance = FVector::Dist(CurrentPosition, TargetPosition);

	// Check if we're close enough to stop smoothing
	if (Distance < 1.0f)
	{
		Owner->SetActorLocation(TargetPosition);
		bIsSmoothing = false;
		return;
	}

	// Interpolate towards target
	FVector NewPosition = FMath::VInterpTo(CurrentPosition, TargetPosition, DeltaTime, SmoothingSpeed);
	Owner->SetActorLocation(NewPosition);

	// Track reconciled path
	ReconciledPath.Add(NewPosition);
	if (ReconciledPath.Num() > MaxPathPoints)
	{
		ReconciledPath.RemoveAt(0);
	}
}

void UNetworkReconciler::DrawDebugPaths()
{
	UWorld* World = GetWorld();
	if (!World || ReconciledPath.Num() < 2)
	{
		return;
	}

	// Draw reconciled path in red
	for (int32 i = 0; i < ReconciledPath.Num() - 1; ++i)
	{
		DrawDebugLine(World, ReconciledPath[i], ReconciledPath[i + 1], FColor::Red, false, 0.1f, 0, 3.0f);
	}

	// Draw correction marker at last correction point
	if (TotalCorrections > 0 && !LastCorrectionDelta.IsNearlyZero())
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			FVector CorrectionPoint = Owner->GetActorLocation() - LastCorrectionDelta;
			DrawDebugSphere(World, CorrectionPoint, 30.0f, 12, FColor::Orange, false, 0.1f, 0, 2.0f);
			DrawDebugLine(World, CorrectionPoint, Owner->GetActorLocation(), FColor::Orange, false, 0.1f, 0, 4.0f);
		}
	}
}
