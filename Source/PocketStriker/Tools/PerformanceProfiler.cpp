// Copyright Epic Games, Inc. All Rights Reserved.

#include "PerformanceProfiler.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/Engine.h"

// Static instance for console commands
UPerformanceProfiler* UPerformanceProfiler::ActiveProfiler = nullptr;

UPerformanceProfiler::UPerformanceProfiler()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UPerformanceProfiler::BeginPlay()
{
	Super::BeginPlay();
	
	// Set as active profiler for console commands
	ActiveProfiler = this;
	
	// Register console commands
	RegisterConsoleCommands();
}

void UPerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsProfiling)
	{
		RecordFrameTime(DeltaTime);
	}
}

void UPerformanceProfiler::StartProfiling()
{
	if (!bIsProfiling)
	{
		bIsProfiling = true;
		ProfilingStartTime = FPlatformTime::Seconds();
		ResetStats();
		
		UE_LOG(LogTemp, Log, TEXT("Performance profiling started"));
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Performance Profiling Started"));
		}
	}
}

void UPerformanceProfiler::StopProfiling()
{
	if (bIsProfiling)
	{
		bIsProfiling = false;
		double Duration = FPlatformTime::Seconds() - ProfilingStartTime;
		
		FFrameTimeStats Stats = GetFrameTimeStats();
		
		UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped. Duration: %.2f seconds"), Duration);
		UE_LOG(LogTemp, Log, TEXT("Frame Stats - Avg: %.2fms, 90th: %.2fms, 99th: %.2fms"), 
			Stats.AverageFrameTime, Stats.Percentile90th, Stats.Percentile99th);
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
				FString::Printf(TEXT("Profiling Stopped - Avg: %.2fms, 90th: %.2fms, 99th: %.2fms"), 
					Stats.AverageFrameTime, Stats.Percentile90th, Stats.Percentile99th));
		}
	}
}

void UPerformanceProfiler::ResetStats()
{
	FrameTimeSamples.Empty();
	TotalFrameTime = 0.0f;
	LatencySamples.Empty();
	PendingInputTimestamps.Empty();
	CorrectionMagnitudes.Empty();
	CorrectionTimestamps.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("Performance profiling stats reset"));
}

void UPerformanceProfiler::RecordFrameTime(float DeltaTime)
{
	if (FrameTimeSamples.Num() >= MaxFrameSamples)
	{
		// Remove oldest sample
		TotalFrameTime -= FrameTimeSamples[0];
		FrameTimeSamples.RemoveAt(0);
	}
	
	float FrameTimeMs = DeltaTime * 1000.0f;
	FrameTimeSamples.Add(FrameTimeMs);
	TotalFrameTime += FrameTimeMs;
}

FFrameTimeStats UPerformanceProfiler::GetFrameTimeStats() const
{
	return CalculateFrameTimeStats();
}

FFrameTimeStats UPerformanceProfiler::CalculateFrameTimeStats() const
{
	FFrameTimeStats Stats;
	
	if (FrameTimeSamples.Num() == 0)
	{
		return Stats;
	}
	
	// Create sorted copy for percentile calculation
	TArray<float> SortedSamples = FrameTimeSamples;
	SortedSamples.Sort();
	
	Stats.TotalFrames = SortedSamples.Num();
	Stats.MinFrameTime = SortedSamples[0];
	Stats.MaxFrameTime = SortedSamples[SortedSamples.Num() - 1];
	Stats.AverageFrameTime = TotalFrameTime / Stats.TotalFrames;
	Stats.Percentile50th = CalculatePercentile(SortedSamples, 0.50f);
	Stats.Percentile90th = CalculatePercentile(SortedSamples, 0.90f);
	Stats.Percentile99th = CalculatePercentile(SortedSamples, 0.99f);
	
	return Stats;
}

float UPerformanceProfiler::CalculatePercentile(const TArray<float>& SortedData, float Percentile) const
{
	if (SortedData.Num() == 0)
	{
		return 0.0f;
	}
	
	int32 Index = FMath::FloorToInt(Percentile * (SortedData.Num() - 1));
	Index = FMath::Clamp(Index, 0, SortedData.Num() - 1);
	
	return SortedData[Index];
}

void UPerformanceProfiler::RecordInputTimestamp(double Timestamp)
{
	if (bIsProfiling)
	{
		PendingInputTimestamps.Add(Timestamp);
	}
}

void UPerformanceProfiler::RecordDisplayTimestamp(double Timestamp)
{
	if (bIsProfiling && PendingInputTimestamps.Num() > 0)
	{
		// Match with oldest pending input
		double InputTimestamp = PendingInputTimestamps[0];
		PendingInputTimestamps.RemoveAt(0);
		
		FInputLatencyMeasurement Measurement;
		Measurement.InputTimestamp = InputTimestamp;
		Measurement.DisplayTimestamp = Timestamp;
		Measurement.LatencyMs = (Timestamp - InputTimestamp) * 1000.0f;
		
		if (LatencySamples.Num() >= MaxLatencySamples)
		{
			LatencySamples.RemoveAt(0);
		}
		
		LatencySamples.Add(Measurement);
	}
}

float UPerformanceProfiler::GetAverageInputLatency() const
{
	if (LatencySamples.Num() == 0)
	{
		return 0.0f;
	}
	
	float Total = 0.0f;
	for (const FInputLatencyMeasurement& Sample : LatencySamples)
	{
		Total += Sample.LatencyMs;
	}
	
	return Total / LatencySamples.Num();
}

float UPerformanceProfiler::GetMaxInputLatency() const
{
	if (LatencySamples.Num() == 0)
	{
		return 0.0f;
	}
	
	float Max = 0.0f;
	for (const FInputLatencyMeasurement& Sample : LatencySamples)
	{
		Max = FMath::Max(Max, Sample.LatencyMs);
	}
	
	return Max;
}

void UPerformanceProfiler::RecordNetworkCorrection(float CorrectionMagnitude)
{
	if (bIsProfiling)
	{
		CorrectionMagnitudes.Add(CorrectionMagnitude);
		CorrectionTimestamps.Add(FPlatformTime::Seconds());
	}
}

FNetworkCorrectionStats UPerformanceProfiler::GetNetworkCorrectionStats() const
{
	return CalculateNetworkStats();
}

FNetworkCorrectionStats UPerformanceProfiler::CalculateNetworkStats() const
{
	FNetworkCorrectionStats Stats;
	
	if (CorrectionMagnitudes.Num() == 0)
	{
		return Stats;
	}
	
	Stats.TotalCorrections = CorrectionMagnitudes.Num();
	
	float Total = 0.0f;
	Stats.MaxCorrectionMagnitude = 0.0f;
	
	for (float Magnitude : CorrectionMagnitudes)
	{
		Total += Magnitude;
		Stats.MaxCorrectionMagnitude = FMath::Max(Stats.MaxCorrectionMagnitude, Magnitude);
	}
	
	Stats.AverageCorrectionMagnitude = Total / Stats.TotalCorrections;
	
	// Calculate frequency (corrections per second)
	if (CorrectionTimestamps.Num() >= 2)
	{
		double Duration = CorrectionTimestamps.Last() - CorrectionTimestamps[0];
		if (Duration > 0.0)
		{
			Stats.CorrectionFrequency = Stats.TotalCorrections / Duration;
		}
	}
	
	return Stats;
}

bool UPerformanceProfiler::ExportToCSV(const FString& Filename)
{
	FString CSVContent;
	
	// Header
	CSVContent += TEXT("=== Performance Profiling Report ===\n");
	CSVContent += FString::Printf(TEXT("Export Time: %s\n\n"), *FDateTime::Now().ToString());
	
	// Frame time statistics
	FFrameTimeStats FrameStats = GetFrameTimeStats();
	CSVContent += TEXT("=== Frame Time Statistics ===\n");
	CSVContent += TEXT("Metric,Value (ms)\n");
	CSVContent += FString::Printf(TEXT("Total Frames,%d\n"), FrameStats.TotalFrames);
	CSVContent += FString::Printf(TEXT("Min Frame Time,%.3f\n"), FrameStats.MinFrameTime);
	CSVContent += FString::Printf(TEXT("Max Frame Time,%.3f\n"), FrameStats.MaxFrameTime);
	CSVContent += FString::Printf(TEXT("Average Frame Time,%.3f\n"), FrameStats.AverageFrameTime);
	CSVContent += FString::Printf(TEXT("50th Percentile,%.3f\n"), FrameStats.Percentile50th);
	CSVContent += FString::Printf(TEXT("90th Percentile,%.3f\n"), FrameStats.Percentile90th);
	CSVContent += FString::Printf(TEXT("99th Percentile,%.3f\n"), FrameStats.Percentile99th);
	CSVContent += TEXT("\n");
	
	// Input latency statistics
	CSVContent += TEXT("=== Input Latency Statistics ===\n");
	CSVContent += TEXT("Metric,Value (ms)\n");
	CSVContent += FString::Printf(TEXT("Total Samples,%d\n"), LatencySamples.Num());
	CSVContent += FString::Printf(TEXT("Average Latency,%.3f\n"), GetAverageInputLatency());
	CSVContent += FString::Printf(TEXT("Max Latency,%.3f\n"), GetMaxInputLatency());
	CSVContent += TEXT("\n");
	
	// Network correction statistics
	FNetworkCorrectionStats NetStats = GetNetworkCorrectionStats();
	CSVContent += TEXT("=== Network Correction Statistics ===\n");
	CSVContent += TEXT("Metric,Value\n");
	CSVContent += FString::Printf(TEXT("Total Corrections,%d\n"), NetStats.TotalCorrections);
	CSVContent += FString::Printf(TEXT("Average Magnitude (cm),%.3f\n"), NetStats.AverageCorrectionMagnitude);
	CSVContent += FString::Printf(TEXT("Max Magnitude (cm),%.3f\n"), NetStats.MaxCorrectionMagnitude);
	CSVContent += FString::Printf(TEXT("Correction Frequency (per sec),%.3f\n"), NetStats.CorrectionFrequency);
	CSVContent += TEXT("\n");
	
	// Raw frame time data
	CSVContent += TEXT("=== Raw Frame Time Data ===\n");
	CSVContent += TEXT("Frame,Time (ms)\n");
	for (int32 i = 0; i < FrameTimeSamples.Num(); ++i)
	{
		CSVContent += FString::Printf(TEXT("%d,%.3f\n"), i, FrameTimeSamples[i]);
	}
	CSVContent += TEXT("\n");
	
	// Raw latency data
	CSVContent += TEXT("=== Raw Input Latency Data ===\n");
	CSVContent += TEXT("Sample,Latency (ms)\n");
	for (int32 i = 0; i < LatencySamples.Num(); ++i)
	{
		CSVContent += FString::Printf(TEXT("%d,%.3f\n"), i, LatencySamples[i].LatencyMs);
	}
	
	// Save to file
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("Profiling") / Filename;
	
	if (FFileHelper::SaveStringToFile(CSVContent, *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("Performance data exported to: %s"), *FilePath);
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
				FString::Printf(TEXT("Profiling data exported to: %s"), *FilePath));
		}
		
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to export performance data to: %s"), *FilePath);
		
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Failed to export profiling data"));
		}
		
		return false;
	}
}

void UPerformanceProfiler::RegisterConsoleCommands()
{
	static bool bCommandsRegistered = false;
	
	if (!bCommandsRegistered)
	{
		IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("perf.start"),
			TEXT("Start performance profiling"),
			FConsoleCommandWithArgsDelegate::CreateStatic(&UPerformanceProfiler::StartProfilingCommand),
			ECVF_Default
		);
		
		IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("perf.stop"),
			TEXT("Stop performance profiling"),
			FConsoleCommandWithArgsDelegate::CreateStatic(&UPerformanceProfiler::StopProfilingCommand),
			ECVF_Default
		);
		
		IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("perf.export"),
			TEXT("Export profiling data to CSV. Usage: perf.export [filename]"),
			FConsoleCommandWithArgsDelegate::CreateStatic(&UPerformanceProfiler::ExportProfilingDataCommand),
			ECVF_Default
		);
		
		IConsoleManager::Get().RegisterConsoleCommand(
			TEXT("perf.reset"),
			TEXT("Reset profiling statistics"),
			FConsoleCommandWithArgsDelegate::CreateStatic(&UPerformanceProfiler::ResetProfilingStatsCommand),
			ECVF_Default
		);
		
		bCommandsRegistered = true;
		UE_LOG(LogTemp, Log, TEXT("Performance profiler console commands registered"));
	}
}

void UPerformanceProfiler::StartProfilingCommand(const TArray<FString>& Args)
{
	if (ActiveProfiler)
	{
		ActiveProfiler->StartProfiling();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No active performance profiler found"));
	}
}

void UPerformanceProfiler::StopProfilingCommand(const TArray<FString>& Args)
{
	if (ActiveProfiler)
	{
		ActiveProfiler->StopProfiling();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No active performance profiler found"));
	}
}

void UPerformanceProfiler::ExportProfilingDataCommand(const TArray<FString>& Args)
{
	if (ActiveProfiler)
	{
		FString Filename = TEXT("performance_data.csv");
		
		if (Args.Num() > 0)
		{
			Filename = Args[0];
			if (!Filename.EndsWith(TEXT(".csv")))
			{
				Filename += TEXT(".csv");
			}
		}
		
		ActiveProfiler->ExportToCSV(Filename);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No active performance profiler found"));
	}
}

void UPerformanceProfiler::ResetProfilingStatsCommand(const TArray<FString>& Args)
{
	if (ActiveProfiler)
	{
		ActiveProfiler->ResetStats();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No active performance profiler found"));
	}
}
