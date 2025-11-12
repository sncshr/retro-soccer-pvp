// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PerformanceProfiler.generated.h"

/**
 * Frame time statistics structure
 */
USTRUCT(BlueprintType)
struct FFrameTimeStats
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	float MinFrameTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	float MaxFrameTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	float AverageFrameTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	float Percentile50th = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	float Percentile90th = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	float Percentile99th = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 TotalFrames = 0;
};

/**
 * Input latency measurement structure
 */
USTRUCT(BlueprintType)
struct FInputLatencyMeasurement
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	double InputTimestamp = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	double DisplayTimestamp = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	float LatencyMs = 0.0f;
};

/**
 * Network correction statistics
 */
USTRUCT(BlueprintType)
struct FNetworkCorrectionStats
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 TotalCorrections = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	float AverageCorrectionMagnitude = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	float MaxCorrectionMagnitude = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	float CorrectionFrequency = 0.0f; // Corrections per second
};

/**
 * Performance profiler component
 * Tracks frame time statistics, input latency, and network correction metrics
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POCKETSTRIKER_API UPerformanceProfiler : public UActorComponent
{
	GENERATED_BODY()

public:
	UPerformanceProfiler();

	// Profiling control
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void StartProfiling();

	UFUNCTION(BlueprintCallable, Category = "Performance")
	void StopProfiling();

	UFUNCTION(BlueprintCallable, Category = "Performance")
	bool IsProfiling() const { return bIsProfiling; }

	UFUNCTION(BlueprintCallable, Category = "Performance")
	void ResetStats();

	// Frame time tracking
	void RecordFrameTime(float DeltaTime);
	
	UFUNCTION(BlueprintCallable, Category = "Performance")
	FFrameTimeStats GetFrameTimeStats() const;

	// Input latency tracking
	void RecordInputTimestamp(double Timestamp);
	void RecordDisplayTimestamp(double Timestamp);
	
	UFUNCTION(BlueprintCallable, Category = "Performance")
	float GetAverageInputLatency() const;

	UFUNCTION(BlueprintCallable, Category = "Performance")
	float GetMaxInputLatency() const;

	// Network correction tracking
	void RecordNetworkCorrection(float CorrectionMagnitude);
	
	UFUNCTION(BlueprintCallable, Category = "Performance")
	FNetworkCorrectionStats GetNetworkCorrectionStats() const;

	// CSV export
	UFUNCTION(BlueprintCallable, Category = "Performance")
	bool ExportToCSV(const FString& Filename);

	// Console command registration
	static void RegisterConsoleCommands();

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ToolTip = "Maximum number of frame time samples to store"))
	int32 MaxFrameSamples = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ToolTip = "Maximum number of latency samples to store"))
	int32 MaxLatencySamples = 1000;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Profiling state
	bool bIsProfiling = false;
	double ProfilingStartTime = 0.0;

	// Frame time data
	TArray<float> FrameTimeSamples;
	float TotalFrameTime = 0.0f;

	// Input latency data
	TArray<FInputLatencyMeasurement> LatencySamples;
	TArray<double> PendingInputTimestamps;

	// Network correction data
	TArray<float> CorrectionMagnitudes;
	TArray<double> CorrectionTimestamps;

	// Helper functions
	FFrameTimeStats CalculateFrameTimeStats() const;
	float CalculatePercentile(const TArray<float>& SortedData, float Percentile) const;
	FNetworkCorrectionStats CalculateNetworkStats() const;

	// Console command callbacks
	static void StartProfilingCommand(const TArray<FString>& Args);
	static void StopProfilingCommand(const TArray<FString>& Args);
	static void ExportProfilingDataCommand(const TArray<FString>& Args);
	static void ResetProfilingStatsCommand(const TArray<FString>& Args);

	// Static instance for console commands
	static UPerformanceProfiler* ActiveProfiler;
};
