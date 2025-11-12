// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Serialization/Archive.h"
#include "NetworkTypes.generated.h"

/**
 * Input packet structure for client-to-server communication
 * Contains all input data with sequence numbering for prediction
 */
USTRUCT(BlueprintType)
struct FInputPacket
{
	GENERATED_BODY()

	UPROPERTY()
	uint32 SequenceNumber = 0;

	UPROPERTY()
	float ClientTimestamp = 0.0f;

	UPROPERTY()
	FVector2D MovementInput = FVector2D::ZeroVector;

	UPROPERTY()
	FVector2D LookInput = FVector2D::ZeroVector;

	UPROPERTY()
	uint32 ActionFlags = 0; // Bitfield for button states

	UPROPERTY()
	uint32 Checksum = 0; // For packet integrity validation

	// Action flag bits
	static constexpr uint32 FLAG_SPRINT = 1 << 0;
	static constexpr uint32 FLAG_TACKLE = 1 << 1;
	static constexpr uint32 FLAG_KICK = 1 << 2;
	static constexpr uint32 FLAG_PASS = 1 << 3;

	FInputPacket()
		: SequenceNumber(0)
		, ClientTimestamp(0.0f)
		, MovementInput(FVector2D::ZeroVector)
		, LookInput(FVector2D::ZeroVector)
		, ActionFlags(0)
		, Checksum(0)
	{
	}

	// Serialization for network transmission
	bool Serialize(FArchive& Ar);

	// Calculate checksum for validation
	uint32 CalculateChecksum() const;

	// Validate packet integrity
	bool IsValid() const;
};

/**
 * State update packet structure for server-to-client communication
 * Contains authoritative state with acknowledgment
 */
USTRUCT(BlueprintType)
struct FStateUpdatePacket
{
	GENERATED_BODY()

	UPROPERTY()
	uint32 AcknowledgedSequence = 0;

	UPROPERTY()
	float ServerTimestamp = 0.0f;

	UPROPERTY()
	FVector AuthoritativePosition = FVector::ZeroVector;

	UPROPERTY()
	FVector AuthoritativeVelocity = FVector::ZeroVector;

	UPROPERTY()
	uint8 AuthoritativeState = 0; // EPlayerState as uint8

	UPROPERTY()
	float AuthoritativeStamina = 100.0f;

	UPROPERTY()
	uint32 Checksum = 0; // For packet integrity validation

	FStateUpdatePacket()
		: AcknowledgedSequence(0)
		, ServerTimestamp(0.0f)
		, AuthoritativePosition(FVector::ZeroVector)
		, AuthoritativeVelocity(FVector::ZeroVector)
		, AuthoritativeState(0)
		, AuthoritativeStamina(100.0f)
		, Checksum(0)
	{
	}

	// Serialization for network transmission
	bool Serialize(FArchive& Ar);

	// Calculate checksum for validation
	uint32 CalculateChecksum() const;

	// Validate packet integrity
	bool IsValid() const;
};

/**
 * Prediction state structure for client-side state history
 * Used for reconciliation after server corrections
 */
USTRUCT(BlueprintType)
struct FPredictionState
{
	GENERATED_BODY()

	UPROPERTY()
	uint32 SequenceNumber = 0;

	UPROPERTY()
	FVector Position = FVector::ZeroVector;

	UPROPERTY()
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY()
	uint8 State = 0; // EPlayerState as uint8

	UPROPERTY()
	float Stamina = 100.0f;

	UPROPERTY()
	float Timestamp = 0.0f;

	FPredictionState()
		: SequenceNumber(0)
		, Position(FVector::ZeroVector)
		, Velocity(FVector::ZeroVector)
		, State(0)
		, Stamina(100.0f)
		, Timestamp(0.0f)
	{
	}
};

// Template specialization for FArchive serialization
template<>
struct TStructOpsTypeTraits<FInputPacket> : public TStructOpsTypeTraitsBase2<FInputPacket>
{
	enum
	{
		WithSerializer = true
	};
};

template<>
struct TStructOpsTypeTraits<FStateUpdatePacket> : public TStructOpsTypeTraitsBase2<FStateUpdatePacket>
{
	enum
	{
		WithSerializer = true
	};
};
