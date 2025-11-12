// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MotionDatabaseEditorUtility.generated.h"

class UMotionDatabase;
class UAnimSequence;

/**
 * Editor utility for motion database preprocessing
 * Provides helper functions for setting up and processing motion databases
 */
UCLASS()
class POCKETSTRIKER_API UMotionDatabaseEditorUtility : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Preprocess a motion database asset
	 * Extracts features from all source animations and builds search index
	 * @param Database The motion database to preprocess
	 * @return True if preprocessing succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	static bool PreprocessMotionDatabase(UMotionDatabase* Database);

	/**
	 * Validate that all animations in a database use the same skeleton
	 * @param Database The motion database to validate
	 * @return True if all animations are compatible
	 */
	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	static bool ValidateAnimationSkeleton(UMotionDatabase* Database);

	/**
	 * Get statistics about a motion database
	 * @param Database The motion database to analyze
	 * @param OutFrameCount Total number of indexed frames
	 * @param OutAnimationCount Number of source animations
	 * @param OutMemorySize Estimated memory size in bytes
	 */
	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	static void GetDatabaseStatistics(UMotionDatabase* Database, int32& OutFrameCount, int32& OutAnimationCount, int32& OutMemorySize);

	/**
	 * Clear all preprocessed data from a database
	 * @param Database The motion database to clear
	 */
	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	static void ClearDatabaseCache(UMotionDatabase* Database);

	/**
	 * Auto-tag animations based on their names
	 * Attempts to assign appropriate action tags by parsing animation names
	 * @param Database The motion database to auto-tag
	 */
	UFUNCTION(BlueprintCallable, Category = "Motion Matching")
	static void AutoTagAnimations(UMotionDatabase* Database);

private:
	/** Helper to determine action tag from animation name */
	static EActionTag DetermineActionTagFromName(const FString& AnimationName);
};
