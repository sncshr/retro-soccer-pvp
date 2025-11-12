// Copyright Epic Games, Inc. All Rights Reserved.

#include "PocketStrikerDebugHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "../Network/NetworkDebugger.h"
#include "../Network/NetworkPrediction.h"
#include "../Network/NetworkReconciler.h"
#include "../AI/AIControllerFootball.h"
#include "../AI/FootballAIUtility.h"
#include "../Animation/MotionMatcher.h"
#include "PerformanceProfiler.h"

APocketStrikerDebugHUD::APocketStrikerDebugHUD()
{
	bShowDebugInfo = true;
}

void APocketStrikerDebugHUD::BeginPlay()
{
	Super::BeginPlay();
}

void APocketStrikerDebugHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!bShowDebugInfo || !Canvas)
	{
		return;
	}

	// Draw all debug sections
	DrawFPSCounter(Canvas);
	DrawFrameTimings(Canvas);
	DrawNetworkStats(Canvas);
	DrawLastCorrection(Canvas);
	DrawAIStates(Canvas);
	DrawMotionMatchingInfo(Canvas);
	DrawInputBuffer(Canvas);
	
	// Draw network path visualizations (3D)
	if (bShowNetworkPaths)
	{
		DrawPredictedPath(Canvas);
		DrawReconciledPath(Canvas);
	}
	
	// Draw collision hitboxes (3D)
	if (bShowHitboxes)
	{
		DrawHitboxes(Canvas);
	}
}

void APocketStrikerDebugHUD::DrawFPSCounter(UCanvas* InCanvas)
{
	if (!InCanvas)
	{
		return;
	}

	// Get FPS from engine
	float FPS = 1.0f / GetWorld()->GetDeltaSeconds();
	
	// Color code based on performance
	FLinearColor Color = FLinearColor::Green;
	if (FPS < 60.0f)
	{
		Color = FLinearColor::Yellow;
	}
	if (FPS < 30.0f)
	{
		Color = FLinearColor::Red;
	}

	// Draw FPS counter
	FString FPSText = FString::Printf(TEXT("FPS: %.1f"), FPS);
	DrawText(FPSText, Color, 10.0f, 10.0f, nullptr, 1.5f);
}

void APocketStrikerDebugHUD::DrawFrameTimings(UCanvas* InCanvas)
{
	if (!InCanvas)
	{
		return;
	}

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	float FrameTimeMs = DeltaTime * 1000.0f;

	// Color code based on frame time
	FLinearColor Color = FLinearColor::Green;
	if (FrameTimeMs > 16.7f) // 60 FPS threshold
	{
		Color = FLinearColor::Yellow;
	}
	if (FrameTimeMs > 33.3f) // 30 FPS threshold
	{
		Color = FLinearColor::Red;
	}

	// Draw frame time
	FString FrameTimeText = FString::Printf(TEXT("Frame Time: %.2f ms"), FrameTimeMs);
	DrawText(FrameTimeText, Color, 10.0f, 40.0f, nullptr, 1.2f);

	// Draw target line
	FString TargetText = TEXT("Target: 16.7 ms (60 FPS)");
	DrawText(TargetText, FLinearColor::Gray, 10.0f, 65.0f, nullptr, 1.0f);

	// Try to find performance profiler and display stats
	UPerformanceProfiler* Profiler = nullptr;
	if (APlayerController* PC = GetOwningPlayerController())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			Profiler = Pawn->FindComponentByClass<UPerformanceProfiler>();
		}
	}

	if (Profiler && Profiler->IsProfiling())
	{
		FFrameTimeStats Stats = Profiler->GetFrameTimeStats();
		
		// Draw profiler stats
		float YPos = 90.0f;
		DrawText(TEXT("=== PROFILER ==="), FLinearColor::Cyan, 10.0f, YPos, nullptr, 1.2f);
		YPos += 20.0f;
		
		FString AvgText = FString::Printf(TEXT("Avg: %.2f ms"), Stats.AverageFrameTime);
		DrawText(AvgText, FLinearColor::White, 10.0f, YPos, nullptr, 1.0f);
		YPos += 18.0f;
		
		FString P90Text = FString::Printf(TEXT("90th: %.2f ms"), Stats.Percentile90th);
		FLinearColor P90Color = Stats.Percentile90th <= 16.7f ? FLinearColor::Green : FLinearColor::Red;
		DrawText(P90Text, P90Color, 10.0f, YPos, nullptr, 1.0f);
		YPos += 18.0f;
		
		FString P99Text = FString::Printf(TEXT("99th: %.2f ms"), Stats.Percentile99th);
		DrawText(P99Text, FLinearColor::White, 10.0f, YPos, nullptr, 1.0f);
		YPos += 18.0f;
		
		// Input latency
		float AvgLatency = Profiler->GetAverageInputLatency();
		FString LatencyText = FString::Printf(TEXT("Input Latency: %.1f ms"), AvgLatency);
		FLinearColor LatencyColor = AvgLatency < 100.0f ? FLinearColor::Green : FLinearColor::Yellow;
		DrawText(LatencyText, LatencyColor, 10.0f, YPos, nullptr, 1.0f);
		YPos += 18.0f;
		
		// Network corrections
		FNetworkCorrectionStats NetStats = Profiler->GetNetworkCorrectionStats();
		if (NetStats.TotalCorrections > 0)
		{
			FString CorrectionText = FString::Printf(TEXT("Corrections: %d (%.1f/s)"), 
				NetStats.TotalCorrections, NetStats.CorrectionFrequency);
			DrawText(CorrectionText, FLinearColor::Yellow, 10.0f, YPos, nullptr, 1.0f);
		}
	}
}

void APocketStrikerDebugHUD::DrawNetworkStats(UCanvas* InCanvas)
{
	if (!InCanvas)
	{
		return;
	}

	// Try to find network debugger in the world
	UNetworkDebugger* NetworkDebugger = nullptr;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor)
		{
			TArray<UActorComponent*> Components;
			Actor->GetComponents(UNetworkDebugger::StaticClass(), Components);
			if (Components.Num() > 0)
			{
				NetworkDebugger = Cast<UNetworkDebugger>(Components[0]);
				break;
			}
		}
	}

	float YPos = 100.0f;
	
	if (NetworkDebugger)
	{
		FNetworkStats Stats = NetworkDebugger->GetNetworkStats();
		
		// Draw network section header
		DrawText(TEXT("=== NETWORK ==="), FLinearColor::White, 10.0f, YPos, nullptr, 1.3f);
		YPos += 25.0f;

		// RTT
		FLinearColor RTTColor = Stats.AverageRTT < 100.0f ? FLinearColor::Green : 
		                        Stats.AverageRTT < 200.0f ? FLinearColor::Yellow : FLinearColor::Red;
		FString RTTText = FString::Printf(TEXT("RTT: %.1f ms"), Stats.AverageRTT);
		DrawText(RTTText, RTTColor, 10.0f, YPos, nullptr, 1.1f);
		YPos += 20.0f;

		// Packet loss
		FLinearColor LossColor = Stats.PacketLoss < 1.0f ? FLinearColor::Green :
		                         Stats.PacketLoss < 5.0f ? FLinearColor::Yellow : FLinearColor::Red;
		FString LossText = FString::Printf(TEXT("Packet Loss: %.2f%%"), Stats.PacketLoss);
		DrawText(LossText, LossColor, 10.0f, YPos, nullptr, 1.1f);
		YPos += 20.0f;

		// Correction frequency
		FString CorrectionText = FString::Printf(TEXT("Corrections: %d"), Stats.TotalCorrections);
		DrawText(CorrectionText, FLinearColor::Cyan, 10.0f, YPos, nullptr, 1.1f);
		YPos += 20.0f;

		// Packets sent/received
		FString PacketText = FString::Printf(TEXT("Packets: %d sent / %d recv"), 
			Stats.TotalPacketsSent, Stats.TotalPacketsReceived);
		DrawText(PacketText, FLinearColor::Gray, 10.0f, YPos, nullptr, 0.9f);
	}
	else
	{
		DrawText(TEXT("=== NETWORK ==="), FLinearColor::White, 10.0f, YPos, nullptr, 1.3f);
		YPos += 25.0f;
		DrawText(TEXT("No network debugger found"), FLinearColor::Gray, 10.0f, YPos, nullptr, 1.0f);
	}
}

void APocketStrikerDebugHUD::DrawLastCorrection(UCanvas* InCanvas)
{
	if (!InCanvas)
	{
		return;
	}

	// Try to find network reconciler
	UNetworkReconciler* Reconciler = nullptr;
	APlayerController* PC = GetOwningPlayerController();
	if (PC && PC->GetPawn())
	{
		Reconciler = PC->GetPawn()->FindComponentByClass<UNetworkReconciler>();
	}

	float YPos = 240.0f;

	if (Reconciler)
	{
		// Get last correction info from network debugger
		UNetworkDebugger* NetworkDebugger = nullptr;
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor)
			{
				TArray<UActorComponent*> Components;
				Actor->GetComponents(UNetworkDebugger::StaticClass(), Components);
				if (Components.Num() > 0)
				{
					NetworkDebugger = Cast<UNetworkDebugger>(Components[0]);
					break;
				}
			}
		}

		if (NetworkDebugger)
		{
			FNetworkStats Stats = NetworkDebugger->GetNetworkStats();
			
			// Color code based on correction magnitude
			FLinearColor Color = Stats.LastCorrectionMagnitude < 10.0f ? FLinearColor::Green :
			                     Stats.LastCorrectionMagnitude < 50.0f ? FLinearColor::Yellow : FLinearColor::Red;
			
			FString CorrectionText = FString::Printf(TEXT("Last Correction: %.2f cm"), Stats.LastCorrectionMagnitude);
			DrawText(CorrectionText, Color, 10.0f, YPos, nullptr, 1.1f);
		}
	}
}

void APocketStrikerDebugHUD::DrawAIStates(UCanvas* InCanvas)
{
	if (!InCanvas)
	{
		return;
	}

	float YPos = 280.0f;
	
	// Draw AI section header
	DrawText(TEXT("=== AI AGENTS ==="), FLinearColor::White, 10.0f, YPos, nullptr, 1.3f);
	YPos += 25.0f;

	// Find all AI controllers
	int32 AICount = 0;
	for (TActorIterator<AAIControllerFootball> It(GetWorld()); It; ++It)
	{
		AAIControllerFootball* AIController = *It;
		if (AIController && AIController->GetPawn())
		{
			AICount++;
			
			// Get behavior name
			FString BehaviorName = TEXT("Unknown");
			switch (AIController->CurrentBehavior)
			{
			case EAIBehavior::Idle: BehaviorName = TEXT("Idle"); break;
			case EAIBehavior::Press: BehaviorName = TEXT("Press"); break;
			case EAIBehavior::Support: BehaviorName = TEXT("Support"); break;
			case EAIBehavior::Intercept: BehaviorName = TEXT("Intercept"); break;
			case EAIBehavior::Cover: BehaviorName = TEXT("Cover"); break;
			case EAIBehavior::Retreat: BehaviorName = TEXT("Retreat"); break;
			}

			// Draw AI info
			FString AIText = FString::Printf(TEXT("AI %d: %s"), AICount, *BehaviorName);
			FLinearColor BehaviorColor = FLinearColor::Cyan;
			if (AIController->CurrentBehavior == EAIBehavior::Press)
			{
				BehaviorColor = FLinearColor::Red;
			}
			else if (AIController->CurrentBehavior == EAIBehavior::Intercept)
			{
				BehaviorColor = FLinearColor::Yellow;
			}
			
			DrawText(AIText, BehaviorColor, 10.0f, YPos, nullptr, 1.0f);
			YPos += 18.0f;
		}
	}

	if (AICount == 0)
	{
		DrawText(TEXT("No AI agents found"), FLinearColor::Gray, 10.0f, YPos, nullptr, 1.0f);
	}
}

void APocketStrikerDebugHUD::DrawAIPerception(UCanvas* InCanvas)
{
	// This is handled by 3D debug visualization in DebugVisualizer
	// Called from AI controller's DrawDebugInfo
}

void APocketStrikerDebugHUD::DrawMotionMatchingInfo(UCanvas* InCanvas)
{
	if (!InCanvas)
	{
		return;
	}

	// Try to find motion matcher on player pawn
	UMotionMatcher* MotionMatcher = nullptr;
	APlayerController* PC = GetOwningPlayerController();
	if (PC && PC->GetPawn() && PC->GetPawn()->GetMesh())
	{
		MotionMatcher = Cast<UMotionMatcher>(PC->GetPawn()->GetMesh()->GetAnimInstance());
	}

	float YPos = 400.0f;
	float XPos = 10.0f;
	
	DrawText(TEXT("=== MOTION MATCHING ==="), FLinearColor::White, XPos, YPos, nullptr, 1.3f);
	YPos += 25.0f;

	if (MotionMatcher)
	{
		// Get current search result
		FMotionSearchResult CurrentResult = MotionMatcher->GetCurrentSearchResult();
		
		// Draw search time with color coding
		float AvgSearchTime = MotionMatcher->GetAverageSearchTime();
		FLinearColor SearchTimeColor = AvgSearchTime < 2.0f ? FLinearColor::Green :
		                               AvgSearchTime < 3.0f ? FLinearColor::Yellow : FLinearColor::Red;
		FString SearchTimeText = FString::Printf(TEXT("Avg Search: %.2f ms"), AvgSearchTime);
		DrawText(SearchTimeText, SearchTimeColor, XPos, YPos, nullptr, 1.0f);
		YPos += 18.0f;

		// Draw last search time
		FString LastSearchText = FString::Printf(TEXT("Last Search: %.2f ms"), CurrentResult.SearchTime);
		DrawText(LastSearchText, FLinearColor::Cyan, XPos, YPos, nullptr, 0.9f);
		YPos += 18.0f;

		// Draw match score
		FString ScoreText = FString::Printf(TEXT("Match Score: %.1f"), CurrentResult.MatchScore);
		DrawText(ScoreText, FLinearColor::Gray, XPos, YPos, nullptr, 0.9f);
		YPos += 18.0f;

		// Draw selected clip info
		if (CurrentResult.BestMatch.SourceSequence)
		{
			FString ClipName = CurrentResult.BestMatch.SourceSequence->GetName();
			FString ClipText = FString::Printf(TEXT("Selected: %s [Frame %d]"), *ClipName, CurrentResult.BestMatch.FrameIndex);
			DrawText(ClipText, FLinearColor::White, XPos, YPos, nullptr, 0.9f);
			YPos += 18.0f;
		}
		else
		{
			DrawText(TEXT("Selected: None"), FLinearColor::Gray, XPos, YPos, nullptr, 0.9f);
			YPos += 18.0f;
		}

		// Draw query feature vector info
		FMotionFeature QueryFeature = MotionMatcher->GetLastQueryFeature();
		DrawText(TEXT("Query Feature:"), FLinearColor::White, XPos, YPos, nullptr, 0.9f);
		YPos += 16.0f;
		
		FString VelocityText = FString::Printf(TEXT("  Vel: (%.0f, %.0f, %.0f) | %.0f cm/s"), 
			QueryFeature.Velocity.X, QueryFeature.Velocity.Y, QueryFeature.Velocity.Z, QueryFeature.Velocity.Size());
		DrawText(VelocityText, FLinearColor::Gray, XPos, YPos, nullptr, 0.8f);
		YPos += 14.0f;

		FString AngleText = FString::Printf(TEXT("  Facing: %.1f deg"), QueryFeature.FacingAngle);
		DrawText(AngleText, FLinearColor::Gray, XPos, YPos, nullptr, 0.8f);
		YPos += 14.0f;

		FString ActionText = FString::Printf(TEXT("  Action: %s"), *UEnum::GetValueAsString(QueryFeature.ActionTag));
		DrawText(ActionText, FLinearColor::Gray, XPos, YPos, nullptr, 0.8f);
		YPos += 16.0f;

		// Draw best match feature vector info
		if (CurrentResult.BestMatch.SourceSequence)
		{
			DrawText(TEXT("Best Match Feature:"), FLinearColor::White, XPos, YPos, nullptr, 0.9f);
			YPos += 16.0f;
			
			FString MatchVelText = FString::Printf(TEXT("  Vel: (%.0f, %.0f, %.0f) | %.0f cm/s"), 
				CurrentResult.BestMatch.Velocity.X, CurrentResult.BestMatch.Velocity.Y, 
				CurrentResult.BestMatch.Velocity.Z, CurrentResult.BestMatch.Velocity.Size());
			DrawText(MatchVelText, FLinearColor::Gray, XPos, YPos, nullptr, 0.8f);
			YPos += 14.0f;

			FString MatchAngleText = FString::Printf(TEXT("  Facing: %.1f deg"), CurrentResult.BestMatch.FacingAngle);
			DrawText(MatchAngleText, FLinearColor::Gray, XPos, YPos, nullptr, 0.8f);
			YPos += 14.0f;

			FString MatchActionText = FString::Printf(TEXT("  Action: %s"), *UEnum::GetValueAsString(CurrentResult.BestMatch.ActionTag));
			DrawText(MatchActionText, FLinearColor::Gray, XPos, YPos, nullptr, 0.8f);
			YPos += 16.0f;
		}

		// Draw async status
		FString AsyncText = MotionMatcher->bUseAsyncSearch ? TEXT("Async: ON") : TEXT("Async: OFF");
		DrawText(AsyncText, FLinearColor::Gray, XPos, YPos, nullptr, 0.9f);
		YPos += 18.0f;

		// Draw fallback status
		FString FallbackText = MotionMatcher->ShouldUseFallback() ? TEXT("Mode: FALLBACK") : TEXT("Mode: MOTION MATCHING");
		FLinearColor FallbackColor = MotionMatcher->ShouldUseFallback() ? FLinearColor::Yellow : FLinearColor::Green;
		DrawText(FallbackText, FallbackColor, XPos, YPos, nullptr, 1.0f);
		YPos += 20.0f;

		// Draw performance graph
		DrawText(TEXT("Search Time History (30 frames):"), FLinearColor::White, XPos, YPos, nullptr, 0.9f);
		YPos += 18.0f;
		
		// Draw bar graph
		TArray<float> SearchTimes = MotionMatcher->GetRecentSearchTimes();
		float BarStartX = XPos;
		float BarY = YPos;
		float BarWidth = 3.0f;
		float BarSpacing = 0.5f;
		float BarMaxHeight = 40.0f;
		float MaxTimeScale = 5.0f; // Scale to 5ms max
		float TargetTime = 2.0f; // 2ms target
		
		// Draw background box
		if (Canvas)
		{
			float GraphWidth = SearchTimes.Num() * (BarWidth + BarSpacing);
			FCanvasTileItem BackgroundBox(FVector2D(BarStartX - 2.0f, BarY - 2.0f), 
				FVector2D(GraphWidth + 4.0f, BarMaxHeight + 4.0f), FLinearColor(0.1f, 0.1f, 0.1f, 0.8f));
			BackgroundBox.BlendMode = SE_BLEND_Translucent;
			Canvas->DrawItem(BackgroundBox);
		}
		
		// Draw target line
		float TargetY = BarY + BarMaxHeight - (TargetTime / MaxTimeScale * BarMaxHeight);
		if (Canvas)
		{
			float GraphWidth = SearchTimes.Num() * (BarWidth + BarSpacing);
			FCanvasLineItem TargetLine(FVector2D(BarStartX, TargetY), FVector2D(BarStartX + GraphWidth, TargetY));
			TargetLine.SetColor(FLinearColor::Yellow);
			Canvas->DrawItem(TargetLine);
		}
		
		// Draw bars for each search time
		for (int32 i = 0; i < SearchTimes.Num(); ++i)
		{
			float SearchTime = SearchTimes[i];
			if (SearchTime > 0.0f)
			{
				float BarHeight = FMath::Clamp(SearchTime / MaxTimeScale, 0.0f, 1.0f) * BarMaxHeight;
				float BarX = BarStartX + i * (BarWidth + BarSpacing);
				float BarTop = BarY + BarMaxHeight - BarHeight;
				
				// Color code based on performance
				FLinearColor BarColor = SearchTime < 2.0f ? FLinearColor::Green :
				                        SearchTime < 3.0f ? FLinearColor::Yellow : FLinearColor::Red;
				
				if (Canvas)
				{
					FCanvasTileItem Bar(FVector2D(BarX, BarTop), FVector2D(BarWidth, BarHeight), BarColor);
					Bar.BlendMode = SE_BLEND_Translucent;
					Canvas->DrawItem(Bar);
				}
			}
		}
		
		YPos += BarMaxHeight + 8.0f;
		
		// Draw scale labels
		FString ScaleText = FString::Printf(TEXT("0ms - %.0fms (Target: %.0fms)"), MaxTimeScale, TargetTime);
		DrawText(ScaleText, FLinearColor::Gray, XPos, YPos, nullptr, 0.7f);
		YPos += 14.0f;

		// Draw candidate scores
		DrawCandidateScores(Canvas);
	}
	else
	{
		DrawText(TEXT("No motion matcher found"), FLinearColor::Gray, XPos, YPos, nullptr, 1.0f);
	}
}

void APocketStrikerDebugHUD::DrawCandidateScores(UCanvas* InCanvas)
{
	if (!InCanvas)
	{
		return;
	}

	// Try to find motion matcher on player pawn
	UMotionMatcher* MotionMatcher = nullptr;
	APlayerController* PC = GetOwningPlayerController();
	if (PC && PC->GetPawn() && PC->GetPawn()->GetMesh())
	{
		MotionMatcher = Cast<UMotionMatcher>(PC->GetPawn()->GetMesh()->GetAnimInstance());
	}

	if (!MotionMatcher)
	{
		return;
	}

	// Get top candidate matches
	TArray<FMotionSearchResult> TopCandidates = MotionMatcher->GetTopCandidates();
	
	if (TopCandidates.Num() == 0)
	{
		return;
	}

	float YPos = 720.0f; // Position below the performance graph
	float XPos = 10.0f;
	
	DrawText(TEXT("=== TOP CANDIDATES ==="), FLinearColor::White, XPos, YPos, nullptr, 1.2f);
	YPos += 22.0f;

	// Draw header
	DrawText(TEXT("Rank  Score    Clip Name                Frame"), FLinearColor::Gray, XPos, YPos, nullptr, 0.8f);
	YPos += 16.0f;

	// Draw each candidate
	for (int32 i = 0; i < TopCandidates.Num(); ++i)
	{
		const FMotionSearchResult& Candidate = TopCandidates[i];
		
		// Color code: best match is green, others fade to gray
		FLinearColor CandidateColor = i == 0 ? FLinearColor::Green :
		                              i == 1 ? FLinearColor(0.5f, 0.8f, 0.5f, 1.0f) :
		                              FLinearColor::Gray;
		
		FString ClipName = Candidate.BestMatch.SourceSequence ? 
			Candidate.BestMatch.SourceSequence->GetName() : TEXT("None");
		
		// Truncate long clip names
		if (ClipName.Len() > 20)
		{
			ClipName = ClipName.Left(17) + TEXT("...");
		}
		
		FString CandidateText = FString::Printf(TEXT("#%d    %.1f    %-20s  %d"), 
			i + 1, Candidate.MatchScore, *ClipName, Candidate.BestMatch.FrameIndex);
		
		DrawText(CandidateText, CandidateColor, XPos, YPos, nullptr, 0.85f);
		YPos += 15.0f;
		
		// Show velocity and action for top 3
		if (i < 3)
		{
			FString DetailText = FString::Printf(TEXT("     Vel: %.0f cm/s, Action: %s"), 
				Candidate.BestMatch.Velocity.Size(),
				*UEnum::GetValueAsString(Candidate.BestMatch.ActionTag));
			DrawText(DetailText, FLinearColor(0.6f, 0.6f, 0.6f, 1.0f), XPos, YPos, nullptr, 0.7f);
			YPos += 13.0f;
		}
	}
}

void APocketStrikerDebugHUD::DrawInputBuffer(UCanvas* InCanvas)
{
	if (!InCanvas)
	{
		return;
	}

	// Try to find network prediction component
	UNetworkPrediction* Prediction = nullptr;
	APlayerController* PC = GetOwningPlayerController();
	if (PC && PC->GetPawn())
	{
		Prediction = PC->GetPawn()->FindComponentByClass<UNetworkPrediction>();
	}

	float YPos = 500.0f;
	float XPos = 10.0f;
	
	DrawText(TEXT("=== INPUT BUFFER ==="), FLinearColor::White, XPos, YPos, nullptr, 1.3f);
	YPos += 25.0f;

	if (Prediction)
	{
		// Get unacknowledged inputs
		TArray<FInputPacket> UnackedInputs = Prediction->GetUnacknowledgedInputs(0);
		
		FString BufferText = FString::Printf(TEXT("Buffered Inputs: %d"), UnackedInputs.Num());
		FLinearColor BufferColor = UnackedInputs.Num() < 10 ? FLinearColor::Green :
		                           UnackedInputs.Num() < 30 ? FLinearColor::Yellow : FLinearColor::Red;
		DrawText(BufferText, BufferColor, XPos, YPos, nullptr, 1.0f);
		YPos += 20.0f;

		// Show detailed input buffer visualization
		if (UnackedInputs.Num() > 0)
		{
			DrawText(TEXT("Recent Inputs:"), FLinearColor::White, XPos, YPos, nullptr, 0.9f);
			YPos += 18.0f;

			// Show last 5 inputs
			int32 NumToShow = FMath::Min(5, UnackedInputs.Num());
			for (int32 i = UnackedInputs.Num() - NumToShow; i < UnackedInputs.Num(); ++i)
			{
				const FInputPacket& Input = UnackedInputs[i];
				
				// Format movement input
				FString MovementStr = FString::Printf(TEXT("(%.1f, %.1f)"), 
					Input.MovementInput.X, Input.MovementInput.Y);
				
				// Format action flags
				FString ActionsStr = TEXT("");
				if (Input.ActionFlags & FInputPacket::FLAG_SPRINT) ActionsStr += TEXT("S");
				if (Input.ActionFlags & FInputPacket::FLAG_TACKLE) ActionsStr += TEXT("T");
				if (Input.ActionFlags & FInputPacket::FLAG_KICK) ActionsStr += TEXT("K");
				if (Input.ActionFlags & FInputPacket::FLAG_PASS) ActionsStr += TEXT("P");
				if (ActionsStr.IsEmpty()) ActionsStr = TEXT("-");
				
				FString InputText = FString::Printf(TEXT("  #%d: Move %s | Actions: %s"), 
					Input.SequenceNumber, *MovementStr, *ActionsStr);
				
				// Color code by age (newer = brighter)
				float Alpha = 0.5f + (0.5f * (float)(i - (UnackedInputs.Num() - NumToShow)) / (float)NumToShow);
				FLinearColor InputColor = FLinearColor(0.7f, 0.7f, 1.0f, Alpha);
				
				DrawText(InputText, InputColor, XPos, YPos, nullptr, 0.8f);
				YPos += 15.0f;
			}
			
			if (UnackedInputs.Num() > NumToShow)
			{
				FString MoreText = FString::Printf(TEXT("  ... and %d more"), UnackedInputs.Num() - NumToShow);
				DrawText(MoreText, FLinearColor::Gray, XPos, YPos, nullptr, 0.7f);
				YPos += 15.0f;
			}
		}
		
		// Show buffer health
		float BufferUsage = (float)UnackedInputs.Num() / (float)Prediction->MaxInputBufferSize;
		FString UsageText = FString::Printf(TEXT("Buffer Usage: %.1f%%"), BufferUsage * 100.0f);
		FLinearColor UsageColor = BufferUsage < 0.5f ? FLinearColor::Green :
		                          BufferUsage < 0.8f ? FLinearColor::Yellow : FLinearColor::Red;
		DrawText(UsageText, UsageColor, XPos, YPos, nullptr, 0.9f);
		YPos += 18.0f;
		
		// Draw visual bar graph of buffer usage
		float BarWidth = 200.0f;
		float BarHeight = 10.0f;
		float BarX = XPos;
		float BarY = YPos;
		
		if (Canvas)
		{
			// Background
			FCanvasTileItem BackgroundBar(FVector2D(BarX, BarY), 
				FVector2D(BarWidth, BarHeight), FLinearColor(0.2f, 0.2f, 0.2f, 0.8f));
			BackgroundBar.BlendMode = SE_BLEND_Translucent;
			Canvas->DrawItem(BackgroundBar);
			
			// Filled portion
			FCanvasTileItem FilledBar(FVector2D(BarX, BarY), 
				FVector2D(BarWidth * BufferUsage, BarHeight), UsageColor);
			FilledBar.BlendMode = SE_BLEND_Translucent;
			Canvas->DrawItem(FilledBar);
		}
		YPos += BarHeight + 5.0f;
	}
	else
	{
		DrawText(TEXT("No prediction component found"), FLinearColor::Gray, XPos, YPos, nullptr, 1.0f);
	}
}

void APocketStrikerDebugHUD::DrawPredictedPath(UCanvas* InCanvas)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Call 3D debug visualization on network prediction component
	APlayerController* PC = GetOwningPlayerController();
	if (PC && PC->GetPawn())
	{
		UNetworkPrediction* Prediction = PC->GetPawn()->FindComponentByClass<UNetworkPrediction>();
		if (Prediction)
		{
			// Get predicted path
			TArray<FVector> PredictedPath = Prediction->GetPredictedPath();
			
			if (PredictedPath.Num() >= 2)
			{
				// Draw predicted path in green with thicker lines
				for (int32 i = 0; i < PredictedPath.Num() - 1; ++i)
				{
					DrawDebugLine(World, PredictedPath[i], PredictedPath[i + 1], 
						FColor::Green, false, 0.1f, 0, 4.0f);
				}

				// Draw position markers along the path
				for (int32 i = 0; i < PredictedPath.Num(); i += 5)
				{
					DrawDebugSphere(World, PredictedPath[i], 10.0f, 8, FColor::Green, false, 0.1f, 0, 1.0f);
				}

				// Draw current position marker (larger)
				FVector CurrentPos = PC->GetPawn()->GetActorLocation();
				DrawDebugSphere(World, CurrentPos, 25.0f, 12, FColor::Green, false, 0.1f, 0, 2.0f);
				
				// Draw label
				FVector LabelPos = CurrentPos + FVector(0, 0, 150.0f);
				DrawDebugString(World, LabelPos, TEXT("PREDICTED"), 
					nullptr, FColor::Green, 0.1f, true, 1.2f);
			}

			// Draw input buffer visualization in 3D
			TArray<FInputPacket> UnackedInputs = Prediction->GetUnacknowledgedInputs(0);
			if (UnackedInputs.Num() > 0)
			{
				FVector BasePos = PC->GetPawn()->GetActorLocation() + FVector(0, 0, 120.0f);
				FString BufferText = FString::Printf(TEXT("Buffered: %d inputs"), UnackedInputs.Num());
				FColor BufferColor = UnackedInputs.Num() < 10 ? FColor::Green :
				                     UnackedInputs.Num() < 30 ? FColor::Yellow : FColor::Red;
				DrawDebugString(World, BasePos, BufferText, nullptr, BufferColor, 0.1f, true, 1.0f);
				
				// Draw input direction indicators
				if (UnackedInputs.Num() > 0)
				{
					const FInputPacket& LatestInput = UnackedInputs.Last();
					if (!LatestInput.MovementInput.IsNearlyZero())
					{
						FVector InputDir = FVector(LatestInput.MovementInput.X, LatestInput.MovementInput.Y, 0).GetSafeNormal();
						FVector ArrowStart = PC->GetPawn()->GetActorLocation() + FVector(0, 0, 50.0f);
						FVector ArrowEnd = ArrowStart + InputDir * 100.0f;
						DrawDebugDirectionalArrow(World, ArrowStart, ArrowEnd, 50.0f, FColor::Cyan, false, 0.1f, 0, 3.0f);
					}
				}
			}
		}
	}
}

void APocketStrikerDebugHUD::DrawReconciledPath(UCanvas* InCanvas)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Call 3D debug visualization on network reconciler component
	APlayerController* PC = GetOwningPlayerController();
	if (PC && PC->GetPawn())
	{
		UNetworkReconciler* Reconciler = PC->GetPawn()->FindComponentByClass<UNetworkReconciler>();
		if (Reconciler)
		{
			// Get reconciled path
			TArray<FVector> ReconciledPath = Reconciler->GetReconciledPath();
			
			if (ReconciledPath.Num() >= 2)
			{
				// Draw reconciled path in red/orange with thicker lines
				for (int32 i = 0; i < ReconciledPath.Num() - 1; ++i)
				{
					DrawDebugLine(World, ReconciledPath[i], ReconciledPath[i + 1], 
						FColor::Orange, false, 0.1f, 0, 4.0f);
				}

				// Draw position markers along the reconciled path
				for (int32 i = 0; i < ReconciledPath.Num(); i += 5)
				{
					DrawDebugSphere(World, ReconciledPath[i], 10.0f, 8, FColor::Orange, false, 0.1f, 0, 1.0f);
				}
			}

			// Draw correction visualization
			if (Reconciler->TotalCorrections > 0 && !Reconciler->LastCorrectionDelta.IsNearlyZero())
			{
				FVector CurrentPos = PC->GetPawn()->GetActorLocation();
				FVector CorrectionPoint = CurrentPos - Reconciler->LastCorrectionDelta;
				
				// Draw correction sphere at the point where correction occurred
				DrawDebugSphere(World, CorrectionPoint, 35.0f, 12, FColor::Red, false, 0.1f, 0, 2.0f);
				
				// Draw arrow showing correction direction and magnitude
				DrawDebugDirectionalArrow(World, CorrectionPoint, CurrentPos, 
					50.0f, FColor::Red, false, 0.1f, 0, 5.0f);
				
				// Draw correction magnitude label
				float CorrectionMagnitude = Reconciler->LastCorrectionDelta.Size();
				FString CorrectionText = FString::Printf(TEXT("Correction: %.1f cm"), CorrectionMagnitude);
				FVector LabelPos = CorrectionPoint + FVector(0, 0, 80.0f);
				DrawDebugString(World, LabelPos, CorrectionText, 
					nullptr, FColor::Red, 0.1f, true, 1.2f);
				
				// Draw time since last correction
				float TimeSinceCorrection = World->GetTimeSeconds() - Reconciler->LastCorrectionTime;
				if (TimeSinceCorrection < 2.0f)
				{
					FString TimeText = FString::Printf(TEXT("%.2fs ago"), TimeSinceCorrection);
					FVector TimePos = LabelPos + FVector(0, 0, 25.0f);
					DrawDebugString(World, TimePos, TimeText, 
						nullptr, FColor::Yellow, 0.1f, true, 0.9f);
				}
			}
			
			// Draw reconciliation status label
			FVector StatusPos = PC->GetPawn()->GetActorLocation() + FVector(0, 0, 180.0f);
			FString StatusText = FString::Printf(TEXT("RECONCILED (Total: %d)"), Reconciler->TotalCorrections);
			DrawDebugString(World, StatusPos, StatusText, 
				nullptr, FColor::Orange, 0.1f, true, 1.0f);
		}
	}
}

void APocketStrikerDebugHUD::DrawHitboxes(UCanvas* InCanvas)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Draw hitboxes for player pawn
	APlayerController* PC = GetOwningPlayerController();
	if (PC && PC->GetPawn())
	{
		// Draw actor bounds
		FVector Origin;
		FVector BoxExtent;
		PC->GetPawn()->GetActorBounds(false, Origin, BoxExtent);
		DrawDebugBox(World, Origin, BoxExtent, FColor::Cyan, false, 0.1f, 0, 2.0f);
		
		// Draw collision capsule if it exists
		ACharacter* Character = Cast<ACharacter>(PC->GetPawn());
		if (Character && Character->GetCapsuleComponent())
		{
			UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
			FVector CapsuleLocation = Capsule->GetComponentLocation();
			float CapsuleRadius = Capsule->GetScaledCapsuleRadius();
			float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
			
			// Draw capsule
			DrawDebugCapsule(World, CapsuleLocation, CapsuleHalfHeight, CapsuleRadius, 
				FQuat::Identity, FColor::Magenta, false, 0.1f, 0, 2.0f);
			
			// Draw capsule dimensions label
			FString DimensionsText = FString::Printf(TEXT("R:%.0f H:%.0f"), CapsuleRadius, CapsuleHalfHeight * 2.0f);
			FVector LabelPos = CapsuleLocation + FVector(0, 0, CapsuleHalfHeight + 30.0f);
			DrawDebugString(World, LabelPos, DimensionsText, 
				nullptr, FColor::Magenta, 0.1f, true, 0.9f);
		}
		
		// Draw mesh bounds if available
		if (Character && Character->GetMesh())
		{
			USkeletalMeshComponent* Mesh = Character->GetMesh();
			FBoxSphereBounds MeshBounds = Mesh->CalcBounds(Mesh->GetComponentTransform());
			DrawDebugBox(World, MeshBounds.Origin, MeshBounds.BoxExtent, 
				FColor::Yellow, false, 0.1f, 0, 1.0f);
		}
		
		// Draw root component location
		FVector RootLocation = PC->GetPawn()->GetActorLocation();
		DrawDebugSphere(World, RootLocation, 15.0f, 8, FColor::White, false, 0.1f, 0, 2.0f);
		DrawDebugCoordinateSystem(World, RootLocation, PC->GetPawn()->GetActorRotation(), 50.0f, false, 0.1f, 0, 2.0f);
	}
	
	// Draw hitboxes for nearby actors (AI, ball, etc.)
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && Actor != PC->GetPawn() && !Actor->IsA<APlayerController>())
		{
			// Only draw for actors within reasonable distance
			float Distance = FVector::Dist(Actor->GetActorLocation(), PC->GetPawn()->GetActorLocation());
			if (Distance < 2000.0f)
			{
				// Draw simple bounds for other actors
				FVector Origin;
				FVector BoxExtent;
				Actor->GetActorBounds(false, Origin, BoxExtent);
				
				// Use different colors for different actor types
				FColor BoundsColor = FColor::Blue;
				if (Actor->GetName().Contains(TEXT("AI")))
				{
					BoundsColor = FColor::Red;
				}
				else if (Actor->GetName().Contains(TEXT("Ball")))
				{
					BoundsColor = FColor::Green;
				}
				
				DrawDebugBox(World, Origin, BoxExtent, BoundsColor, false, 0.1f, 0, 1.0f);
			}
		}
	}
}
