// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NetworkParamsData.generated.h"

/**
 * Data asset for network tuning parameters
 * Allows designers to adjust networking behavior without code changes
 */
UCLASS(BlueprintType)
class POCKETSTRIKER_API UNetworkParamsData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prediction", meta = (ToolTip = "Distance threshold before correction in cm"))
	float CorrectionThreshold = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prediction", meta = (ToolTip = "Speed of correction smoothing"))
	float SmoothingSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interpolation", meta = (ToolTip = "Interpolation delay in seconds"))
	float InterpolationDelay = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interpolation", meta = (ToolTip = "Size of state buffer for interpolation"))
	int32 StateBufferSize = 32;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (ToolTip = "Simulated latency in seconds"))
	float SimulatedLatency = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug", meta = (ToolTip = "Packet loss percentage (0-100)"))
	float PacketLossPercentage = 0.0f;
};
