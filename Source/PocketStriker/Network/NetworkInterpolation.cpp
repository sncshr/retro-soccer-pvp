// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkInterpolation.h"
#include "GameFramework/Character.h"

UNetworkInterpolation::UNetworkInterpolation()
{
	PrimaryComponentTick.bCanEverTick = true;
	LastPacketTime = 0.0f;
	CurrentRenderTime = 0.0f;
}

void UNetworkInterpolation::BeginPlay()
{
	Super::BeginPlay();

	// Reserve space for state buffer
	StateBuffer.Reserve(MaxBufferSize);
	
	// Initialize timing
	if (UWorld* World = GetWorld())
	{
		LastPacketTime = World->GetTimeSeconds();
		CurrentRenderTime = LastPacketTime - InterpolationDelay;
	}
}

void UNetworkInterpolation::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update interpolation
	UpdateInterpolation(DeltaTime);

	// Update debug info
	if (UWorld* World = GetWorld())
	{
		TimeSinceLastPacket = World->GetTimeSeconds() - LastPacketTime;
	}
	CurrentBufferSize = StateBuffer.Num();
}

void UNetworkInterpolation::ApplyNetworkParams(float InInterpolationDelay, int32 InBufferSize)
{
	InterpolationDelay = InInterpolationDelay;
	MaxBufferSize = InBufferSize;
}

void UNetworkInterpolation::AddServerState(const FStateUpdatePacket& State)
{
	// Add state to buffer
	StateBuffer.Add(State);

	// Update last packet time
	if (UWorld* World = GetWorld())
	{
		LastPacketTime = World->GetTimeSeconds();
	}

	// Sort buffer by timestamp (should already be sorted, but ensure it)
	StateBuffer.Sort([](const FStateUpdatePacket& A, const FStateUpdatePacket& B)
	{
		return A.ServerTimestamp < B.ServerTimestamp;
	});

	// Trim old states
	TrimOldStates();
}

void UNetworkInterpolation::UpdateInterpolation(float DeltaTime)
{
	if (StateBuffer.Num() < 2)
	{
		// Not enough states to interpolate
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Advance render time
	CurrentRenderTime += DeltaTime;

	// Get interpolated position
	FVector InterpolatedPosition = GetInterpolatedPosition(CurrentRenderTime);
	
	// Apply to actor
	Owner->SetActorLocation(InterpolatedPosition);
}

FVector UNetworkInterpolation::GetInterpolatedPosition(float RenderTime) const
{
	if (StateBuffer.Num() == 0)
	{
		return FVector::ZeroVector;
	}

	if (StateBuffer.Num() == 1)
	{
		return StateBuffer[0].AuthoritativePosition;
	}

	// Get interpolation states
	FStateUpdatePacket FromState, ToState;
	float Alpha;
	GetInterpolationStates(RenderTime, FromState, ToState, Alpha);

	// Interpolate position
	return FMath::Lerp(FromState.AuthoritativePosition, ToState.AuthoritativePosition, Alpha);
}

FRotator UNetworkInterpolation::GetInterpolatedRotation(float RenderTime) const
{
	if (StateBuffer.Num() == 0)
	{
		return FRotator::ZeroRotator;
	}

	if (StateBuffer.Num() == 1)
	{
		// Calculate rotation from velocity
		FVector Velocity = StateBuffer[0].AuthoritativeVelocity;
		if (!Velocity.IsNearlyZero())
		{
			return Velocity.Rotation();
		}
		return FRotator::ZeroRotator;
	}

	// Get interpolation states
	FStateUpdatePacket FromState, ToState;
	float Alpha;
	GetInterpolationStates(RenderTime, FromState, ToState, Alpha);

	// Calculate rotations from velocities
	FRotator FromRotation = FromState.AuthoritativeVelocity.IsNearlyZero() ? 
		FRotator::ZeroRotator : FromState.AuthoritativeVelocity.Rotation();
	FRotator ToRotation = ToState.AuthoritativeVelocity.IsNearlyZero() ? 
		FRotator::ZeroRotator : ToState.AuthoritativeVelocity.Rotation();

	// Interpolate rotation
	return FMath::Lerp(FromRotation, ToRotation, Alpha);
}

FVector UNetworkInterpolation::GetInterpolatedVelocity(float RenderTime) const
{
	if (StateBuffer.Num() == 0)
	{
		return FVector::ZeroVector;
	}

	if (StateBuffer.Num() == 1)
	{
		return StateBuffer[0].AuthoritativeVelocity;
	}

	// Get interpolation states
	FStateUpdatePacket FromState, ToState;
	float Alpha;
	GetInterpolationStates(RenderTime, FromState, ToState, Alpha);

	// Interpolate velocity
	return FMath::Lerp(FromState.AuthoritativeVelocity, ToState.AuthoritativeVelocity, Alpha);
}

FVector UNetworkInterpolation::ExtrapolatePosition(float DeltaTime)
{
	if (!bEnableExtrapolation || StateBuffer.Num() == 0)
	{
		return FVector::ZeroVector;
	}

	// Use the latest state to extrapolate
	const FStateUpdatePacket& LatestState = StateBuffer.Last();
	
	// Extrapolate using velocity
	FVector ExtrapolatedPosition = LatestState.AuthoritativePosition + 
		(LatestState.AuthoritativeVelocity * DeltaTime);

	return ExtrapolatedPosition;
}

void UNetworkInterpolation::GetInterpolationStates(float RenderTime, FStateUpdatePacket& OutFrom, FStateUpdatePacket& OutTo, float& OutAlpha) const
{
	// Calculate target render time (current time - interpolation delay)
	float TargetTime = RenderTime - InterpolationDelay;

	// Find the two states to interpolate between
	int32 FromIndex = -1;
	int32 ToIndex = -1;

	for (int32 i = 0; i < StateBuffer.Num() - 1; ++i)
	{
		if (StateBuffer[i].ServerTimestamp <= TargetTime && 
			StateBuffer[i + 1].ServerTimestamp >= TargetTime)
		{
			FromIndex = i;
			ToIndex = i + 1;
			break;
		}
	}

	// Handle edge cases
	if (FromIndex == -1)
	{
		// Target time is before all states, use first two
		FromIndex = 0;
		ToIndex = FMath::Min(1, StateBuffer.Num() - 1);
	}

	OutFrom = StateBuffer[FromIndex];
	OutTo = StateBuffer[ToIndex];

	// Calculate interpolation alpha
	float TimeDiff = OutTo.ServerTimestamp - OutFrom.ServerTimestamp;
	if (TimeDiff > 0.0f)
	{
		OutAlpha = (TargetTime - OutFrom.ServerTimestamp) / TimeDiff;
		OutAlpha = FMath::Clamp(OutAlpha, 0.0f, 1.0f);
	}
	else
	{
		OutAlpha = 0.0f;
	}
}

void UNetworkInterpolation::TrimOldStates()
{
	if (StateBuffer.Num() <= MaxBufferSize)
	{
		return;
	}

	// Remove oldest states
	int32 NumToRemove = StateBuffer.Num() - MaxBufferSize;
	StateBuffer.RemoveAt(0, NumToRemove);
}
