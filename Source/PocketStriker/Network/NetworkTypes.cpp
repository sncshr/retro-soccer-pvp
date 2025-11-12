// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkTypes.h"

// Simple checksum calculation using XOR of all data
uint32 FInputPacket::CalculateChecksum() const
{
	uint32 Hash = 0;
	Hash ^= SequenceNumber;
	Hash ^= *reinterpret_cast<const uint32*>(&ClientTimestamp);
	Hash ^= *reinterpret_cast<const uint32*>(&MovementInput.X);
	Hash ^= *reinterpret_cast<const uint32*>(&MovementInput.Y);
	Hash ^= *reinterpret_cast<const uint32*>(&LookInput.X);
	Hash ^= *reinterpret_cast<const uint32*>(&LookInput.Y);
	Hash ^= ActionFlags;
	return Hash;
}

bool FInputPacket::IsValid() const
{
	// Validate checksum
	if (Checksum != CalculateChecksum())
	{
		return false;
	}

	// Validate input ranges
	if (MovementInput.X < -1.0f || MovementInput.X > 1.0f ||
		MovementInput.Y < -1.0f || MovementInput.Y > 1.0f)
	{
		return false;
	}

	if (LookInput.X < -1.0f || LookInput.X > 1.0f ||
		LookInput.Y < -1.0f || LookInput.Y > 1.0f)
	{
		return false;
	}

	return true;
}

bool FInputPacket::Serialize(FArchive& Ar)
{
	Ar << SequenceNumber;
	Ar << ClientTimestamp;
	Ar << MovementInput.X;
	Ar << MovementInput.Y;
	Ar << LookInput.X;
	Ar << LookInput.Y;
	Ar << ActionFlags;
	
	// Calculate checksum on save, validate on load
	if (Ar.IsSaving())
	{
		Checksum = CalculateChecksum();
	}
	
	Ar << Checksum;
	
	if (Ar.IsLoading())
	{
		// Validate after loading
		return IsValid();
	}
	
	return true;
}

uint32 FStateUpdatePacket::CalculateChecksum() const
{
	uint32 Hash = 0;
	Hash ^= AcknowledgedSequence;
	Hash ^= *reinterpret_cast<const uint32*>(&ServerTimestamp);
	Hash ^= *reinterpret_cast<const uint32*>(&AuthoritativePosition.X);
	Hash ^= *reinterpret_cast<const uint32*>(&AuthoritativePosition.Y);
	Hash ^= *reinterpret_cast<const uint32*>(&AuthoritativePosition.Z);
	Hash ^= *reinterpret_cast<const uint32*>(&AuthoritativeVelocity.X);
	Hash ^= *reinterpret_cast<const uint32*>(&AuthoritativeVelocity.Y);
	Hash ^= *reinterpret_cast<const uint32*>(&AuthoritativeVelocity.Z);
	Hash ^= static_cast<uint32>(AuthoritativeState);
	Hash ^= *reinterpret_cast<const uint32*>(&AuthoritativeStamina);
	return Hash;
}

bool FStateUpdatePacket::IsValid() const
{
	// Validate checksum
	if (Checksum != CalculateChecksum())
	{
		return false;
	}

	// Validate stamina range
	if (AuthoritativeStamina < 0.0f || AuthoritativeStamina > 200.0f)
	{
		return false;
	}

	// Validate state enum range (0-5 for EPlayerState)
	if (AuthoritativeState > 5)
	{
		return false;
	}

	return true;
}

bool FStateUpdatePacket::Serialize(FArchive& Ar)
{
	Ar << AcknowledgedSequence;
	Ar << ServerTimestamp;
	Ar << AuthoritativePosition.X;
	Ar << AuthoritativePosition.Y;
	Ar << AuthoritativePosition.Z;
	Ar << AuthoritativeVelocity.X;
	Ar << AuthoritativeVelocity.Y;
	Ar << AuthoritativeVelocity.Z;
	Ar << AuthoritativeState;
	Ar << AuthoritativeStamina;
	
	// Calculate checksum on save, validate on load
	if (Ar.IsSaving())
	{
		Checksum = CalculateChecksum();
	}
	
	Ar << Checksum;
	
	if (Ar.IsLoading())
	{
		// Validate after loading
		return IsValid();
	}
	
	return true;
}
