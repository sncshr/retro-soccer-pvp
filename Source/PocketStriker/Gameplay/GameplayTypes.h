// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTypes.generated.h"

/**
 * Input command structure for network prediction
 * Contains all input data with sequence numbering
 */
USTRUCT(BlueprintType)
struct FInputCommand
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

	// Action flag bits
	static constexpr uint32 FLAG_SPRINT = 1 << 0;
	static constexpr uint32 FLAG_TACKLE = 1 << 1;
	static constexpr uint32 FLAG_KICK = 1 << 2;
	static constexpr uint32 FLAG_PASS = 1 << 3;

	FInputCommand()
		: SequenceNumber(0)
		, ClientTimestamp(0.0f)
		, MovementInput(FVector2D::ZeroVector)
		, LookInput(FVector2D::ZeroVector)
		, ActionFlags(0)
	{
	}
};
