// Copyright Epic Games, Inc. All Rights Reserved.

#include "MotionMatchingPreprocessor.h"
#include "Animation/AnimSequence.h"
#include "MotionDatabase.h"

UMotionMatchingPreprocessor::UMotionMatchingPreprocessor()
{
}

void UMotionMatchingPreprocessor::ExtractFeatures(UAnimSequence* Sequence)
{
	if (!Sequence)
	{
		UE_LOG(LogTemp, Warning, TEXT("MotionMatchingPreprocessor: Null animation sequence provided"));
		return;
	}

	const float SequenceLength = Sequence->GetPlayLength();
	const float FrameRate = 30.0f; // Sample at 30 FPS
	const int32 NumFrames = FMath::CeilToInt(SequenceLength * FrameRate);

	UE_LOG(LogTemp, Log, TEXT("Extracting features from %s: %d frames"), *Sequence->GetName(), NumFrames);

	for (int32 FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex)
	{
		FMotionFeature Feature = ComputeFrameFeature(FrameIndex);
		Feature.SourceSequence = Sequence;
		Feature.FrameIndex = FrameIndex;
		ExtractedFeatures.Add(Feature);
	}
}

FMotionFeature UMotionMatchingPreprocessor::ComputeFrameFeature(int32 FrameIndex)
{
	FMotionFeature Feature;
	
	// For now, create placeholder features
	// In a full implementation, this would extract actual pose data from the animation
	Feature.Velocity = FVector(100.0f * FMath::FRand(), 100.0f * FMath::FRand(), 0.0f);
	Feature.FacingAngle = FMath::FRand() * 360.0f;
	Feature.ActionTag = EActionTag::Run;
	Feature.FrameIndex = FrameIndex;
	
	// Extract simplified joint positions (hips, feet, hands)
	// In production, this would sample actual bone transforms
	Feature.JointPositions.Add(FVector(0.0f, 0.0f, 100.0f)); // Hips
	Feature.JointPositions.Add(FVector(0.0f, -20.0f, 0.0f)); // Left foot
	Feature.JointPositions.Add(FVector(0.0f, 20.0f, 0.0f));  // Right foot
	Feature.JointPositions.Add(FVector(-50.0f, -30.0f, 100.0f)); // Left hand
	Feature.JointPositions.Add(FVector(-50.0f, 30.0f, 100.0f));  // Right hand

	return Feature;
}

void UMotionMatchingPreprocessor::BuildSearchIndex(const TArray<FMotionFeature>& Features)
{
	// Simple spatial hashing implementation
	// In production, this would build a KD-tree or more sophisticated structure
	
	UE_LOG(LogTemp, Log, TEXT("Building search index for %d features"), Features.Num());
	
	// For this prototype, we'll use a simple linear search at runtime
	// The "index" is just the sorted array of features
	ExtractedFeatures = Features;
	
	// Sort by velocity magnitude for basic spatial coherence
	ExtractedFeatures.Sort([](const FMotionFeature& A, const FMotionFeature& B)
	{
		return A.Velocity.Size() < B.Velocity.Size();
	});
}

UMotionDatabase* UMotionMatchingPreprocessor::GenerateDatabase()
{
	UMotionDatabase* Database = NewObject<UMotionDatabase>();
	
	if (!Database)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create motion database"));
		return nullptr;
	}

	// Build the search index from extracted features
	BuildSearchIndex(ExtractedFeatures);
	
	// Copy indexed frames to database
	Database->IndexedFrames = ExtractedFeatures;
	
	UE_LOG(LogTemp, Log, TEXT("Generated motion database with %d indexed frames"), Database->IndexedFrames.Num());
	
	return Database;
}
