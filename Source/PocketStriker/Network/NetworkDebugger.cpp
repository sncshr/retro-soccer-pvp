// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkDebugger.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

UNetworkDebugger::UNetworkDebugger()
{
	SimulatedInboundLatency = 0.0f;
	SimulatedOutboundLatency = 0.0f;
	PacketLossPercentage = 0.0f;
	bEnableDebugDisplay = true;
	bEnableLagSimulation = false;
}

void UNetworkDebugger::SetSimulatedLatency(float InboundMs, float OutboundMs)
{
	SimulatedInboundLatency = FMath::Max(0.0f, InboundMs);
	SimulatedOutboundLatency = FMath::Max(0.0f, OutboundMs);
	bEnableLagSimulation = (SimulatedInboundLatency > 0.0f || SimulatedOutboundLatency > 0.0f);
}

void UNetworkDebugger::SetPacketLossPercentage(float Percentage)
{
	PacketLossPercentage = FMath::Clamp(Percentage, 0.0f, 100.0f);
	bEnableLagSimulation = (PacketLossPercentage > 0.0f);
}

void UNetworkDebugger::DelayPacket(const TArray<uint8>& PacketData, float DelayMs, bool bIsInput)
{
	if (!bEnableLagSimulation || DelayMs <= 0.0f)
	{
		// No delay, process immediately
		return;
	}

	FDelayedPacket DelayedPacket;
	DelayedPacket.PacketData = PacketData;
	DelayedPacket.ReleaseTime = FPlatformTime::Seconds() + (DelayMs / 1000.0f);
	DelayedPacket.bIsInputPacket = bIsInput;

	DelayedPackets.Add(DelayedPacket);
}

void UNetworkDebugger::ProcessDelayedPackets(float DeltaTime)
{
	if (!bEnableLagSimulation)
	{
		return;
	}

	float CurrentTime = FPlatformTime::Seconds();

	// Process packets that are ready to be released
	for (int32 i = DelayedPackets.Num() - 1; i >= 0; --i)
	{
		if (DelayedPackets[i].ReleaseTime <= CurrentTime)
		{
			// Packet is ready to be processed
			// In a real implementation, this would deserialize and process the packet
			// For now, we just remove it from the queue
			
			RecordPacketReceived();
			DelayedPackets.RemoveAt(i);
		}
	}
}

bool UNetworkDebugger::ShouldDropPacket()
{
	if (!bEnableLagSimulation || PacketLossPercentage <= 0.0f)
	{
		return false;
	}

	// Random chance based on packet loss percentage
	float RandomValue = FMath::FRand() * 100.0f;
	bool bDrop = RandomValue < PacketLossPercentage;

	if (bDrop)
	{
		RecordPacketDropped();
	}

	return bDrop;
}

void UNetworkDebugger::RecordPacketSent()
{
	Stats.TotalPacketsSent++;
	UpdatePacketLossPercentage();
}

void UNetworkDebugger::RecordPacketReceived()
{
	Stats.TotalPacketsReceived++;
	UpdatePacketLossPercentage();
}

void UNetworkDebugger::RecordPacketDropped()
{
	Stats.TotalPacketsDropped++;
	UpdatePacketLossPercentage();
}

void UNetworkDebugger::RecordCorrection(float Magnitude)
{
	Stats.TotalCorrections++;
	Stats.LastCorrectionMagnitude = Magnitude;
}

void UNetworkDebugger::UpdateRTT(float RTT)
{
	// Add to samples
	RTTSamples.Add(RTT);

	// Keep only recent samples
	if (RTTSamples.Num() > MaxRTTSamples)
	{
		RTTSamples.RemoveAt(0);
	}

	// Calculate average
	float Sum = 0.0f;
	for (float Sample : RTTSamples)
	{
		Sum += Sample;
	}
	Stats.AverageRTT = RTTSamples.Num() > 0 ? Sum / RTTSamples.Num() : 0.0f;
}

void UNetworkDebugger::UpdatePacketLossPercentage()
{
	int32 TotalPackets = Stats.TotalPacketsSent + Stats.TotalPacketsReceived;
	if (TotalPackets > 0)
	{
		Stats.PacketLoss = (float)Stats.TotalPacketsDropped / (float)TotalPackets * 100.0f;
	}
}

FString UNetworkDebugger::FormatLatency(float Ms) const
{
	return FString::Printf(TEXT("%.1f ms"), Ms);
}

void UNetworkDebugger::DrawNetworkStats(UCanvas* Canvas, float X, float Y)
{
	if (!Canvas || !bEnableDebugDisplay)
	{
		return;
	}

	// Set up text rendering
	FCanvasTextItem TextItem(FVector2D(X, Y), FText::GetEmpty(), GEngine->GetSmallFont(), FLinearColor::White);
	TextItem.EnableShadow(FLinearColor::Black);

	float LineHeight = 16.0f;
	float CurrentY = Y;

	// Title
	TextItem.Text = FText::FromString(TEXT("=== Network Stats ==="));
	TextItem.SetColor(FLinearColor::Yellow);
	Canvas->DrawItem(TextItem, X, CurrentY);
	CurrentY += LineHeight;

	// RTT
	TextItem.SetColor(FLinearColor::White);
	FString RTTText = FString::Printf(TEXT("RTT: %s"), *FormatLatency(Stats.AverageRTT));
	if (bEnableLagSimulation)
	{
		float SimulatedRTT = SimulatedInboundLatency + SimulatedOutboundLatency;
		RTTText += FString::Printf(TEXT(" (Simulated: %s)"), *FormatLatency(SimulatedRTT));
	}
	TextItem.Text = FText::FromString(RTTText);
	Canvas->DrawItem(TextItem, X, CurrentY);
	CurrentY += LineHeight;

	// Packet Loss
	FString PacketLossText = FString::Printf(TEXT("Packet Loss: %.2f%%"), Stats.PacketLoss);
	if (bEnableLagSimulation && PacketLossPercentage > 0.0f)
	{
		PacketLossText += FString::Printf(TEXT(" (Simulated: %.1f%%)"), PacketLossPercentage);
	}
	TextItem.Text = FText::FromString(PacketLossText);
	Canvas->DrawItem(TextItem, X, CurrentY);
	CurrentY += LineHeight;

	// Packets
	FString PacketsText = FString::Printf(TEXT("Packets: Sent=%d Recv=%d Dropped=%d"), 
		Stats.TotalPacketsSent, Stats.TotalPacketsReceived, Stats.TotalPacketsDropped);
	TextItem.Text = FText::FromString(PacketsText);
	Canvas->DrawItem(TextItem, X, CurrentY);
	CurrentY += LineHeight;

	// Corrections
	FString CorrectionsText = FString::Printf(TEXT("Corrections: %d (Last: %.1f cm)"), 
		Stats.TotalCorrections, Stats.LastCorrectionMagnitude);
	TextItem.Text = FText::FromString(CorrectionsText);
	
	// Color code based on correction magnitude
	if (Stats.LastCorrectionMagnitude > 50.0f)
	{
		TextItem.SetColor(FLinearColor::Red);
	}
	else if (Stats.LastCorrectionMagnitude > 20.0f)
	{
		TextItem.SetColor(FLinearColor::Yellow);
	}
	else
	{
		TextItem.SetColor(FLinearColor::Green);
	}
	
	Canvas->DrawItem(TextItem, X, CurrentY);
	CurrentY += LineHeight;

	// Lag simulation status
	if (bEnableLagSimulation)
	{
		TextItem.SetColor(FLinearColor::Yellow);
		TextItem.Text = FText::FromString(TEXT("LAG SIMULATION ACTIVE"));
		Canvas->DrawItem(TextItem, X, CurrentY);
		CurrentY += LineHeight;
	}
}
