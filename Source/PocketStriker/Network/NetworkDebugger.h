// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NetworkTypes.h"
#include "NetworkDebugger.generated.h"

class UCanvas;

/**
 * Delayed packet structure for lag simulation
 */
USTRUCT()
struct FDelayedPacket
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<uint8> PacketData;

	UPROPERTY()
	float ReleaseTime = 0.0f;

	UPROPERTY()
	bool bIsInputPacket = false;

	FDelayedPacket()
		: ReleaseTime(0.0f)
		, bIsInputPacket(false)
	{
	}
};

/**
 * Network statistics tracking
 */
USTRUCT(BlueprintType)
struct FNetworkStats
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float AverageRTT = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float PacketLoss = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 TotalPacketsSent = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 TotalPacketsReceived = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 TotalPacketsDropped = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 TotalCorrections = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float LastCorrectionMagnitude = 0.0f;

	FNetworkStats()
		: AverageRTT(0.0f)
		, PacketLoss(0.0f)
		, TotalPacketsSent(0)
		, TotalPacketsReceived(0)
		, TotalPacketsDropped(0)
		, TotalCorrections(0)
		, LastCorrectionMagnitude(0.0f)
	{
	}
};

/**
 * Network debugger for latency and packet loss simulation
 * Also provides real-time network metrics display
 */
UCLASS(BlueprintType)
class POCKETSTRIKER_API UNetworkDebugger : public UObject
{
	GENERATED_BODY()

public:
	UNetworkDebugger();

	// Lag simulation
	UFUNCTION(BlueprintCallable, Category = "Network Debug")
	void SetSimulatedLatency(float InboundMs, float OutboundMs);

	UFUNCTION(BlueprintCallable, Category = "Network Debug")
	void SetPacketLossPercentage(float Percentage);

	// Packet delay
	void DelayPacket(const TArray<uint8>& PacketData, float DelayMs, bool bIsInput);
	void ProcessDelayedPackets(float DeltaTime);

	// Packet loss
	bool ShouldDropPacket();

	// Statistics
	void RecordPacketSent();
	void RecordPacketReceived();
	void RecordPacketDropped();
	void RecordCorrection(float Magnitude);
	void UpdateRTT(float RTT);

	UFUNCTION(BlueprintCallable, Category = "Network Debug")
	FNetworkStats GetNetworkStats() const { return Stats; }

	// Visualization
	void DrawNetworkStats(UCanvas* Canvas, float X, float Y);

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Debug")
	float SimulatedInboundLatency = 0.0f; // In milliseconds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Debug")
	float SimulatedOutboundLatency = 0.0f; // In milliseconds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Debug")
	float PacketLossPercentage = 0.0f; // 0-100

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Debug")
	bool bEnableDebugDisplay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network Debug")
	bool bEnableLagSimulation = false;

private:
	// Delayed packet queue
	UPROPERTY()
	TArray<FDelayedPacket> DelayedPackets;

	// Statistics
	UPROPERTY()
	FNetworkStats Stats;

	// RTT tracking
	TArray<float> RTTSamples;
	static constexpr int32 MaxRTTSamples = 60;

	// Helper functions
	void UpdatePacketLossPercentage();
	FString FormatLatency(float Ms) const;
};
