// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MotionMatchingPreprocessor.generated.h"

class UAnimSequence;
class UMotionDatabase;
struct FMotionFeature;

/**
 * Preprocessor for motion matching database generation
 * Extracts features from animation sequences and builds search index
 */
UCLASS()
class POCKETSTRIKER_API UMotionMatchingPreprocessor : public UObject
{
	GENERATED_BODY()

public:
	UMotionMatchingPreprocessor();

	// Feature extraction
	void ExtractFeatures(UAnimSequence* Sequence);
	FMotionFeature ComputeFrameFeature(int32 FrameIndex);
	
	// Index building
	void BuildSearchIndex(const TArray<FMotionFeature>& Features);
	
	// Output: Motion database asset
	UMotionDatabase* GenerateDatabase();

protected:
	UPROPERTY()
	TArray<FMotionFeature> ExtractedFeatures;
};
