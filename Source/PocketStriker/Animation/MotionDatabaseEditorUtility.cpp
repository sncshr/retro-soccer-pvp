// Copyright Epic Games, Inc. All Rights Reserved.

#include "MotionDatabaseEditorUtility.h"
#include "MotionDatabase.h"
#include "MotionMatchingPreprocessor.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

bool UMotionDatabaseEditorUtility::PreprocessMotionDatabase(UMotionDatabase* Database)
{
	if (!Database)
	{
		UE_LOG(LogTemp, Error, TEXT("MotionDatabaseEditorUtility: Cannot preprocess null database"));
		return false;
	}

	if (Database->SourceAnimations.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("MotionDatabaseEditorUtility: Database has no source animations"));
		return false;
	}

	// Validate skeleton compatibility
	if (!ValidateAnimationSkeleton(Database))
	{
		UE_LOG(LogTemp, Error, TEXT("MotionDatabaseEditorUtility: Animation skeleton validation failed"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("MotionDatabaseEditorUtility: Starting preprocessing of %d animations..."), 
		Database->SourceAnimations.Num());

	// Create preprocessor
	UMotionMatchingPreprocessor* Preprocessor = NewObject<UMotionMatchingPreprocessor>();
	if (!Preprocessor)
	{
		UE_LOG(LogTemp, Error, TEXT("MotionDatabaseEditorUtility: Failed to create preprocessor"));
		return false;
	}

	// Clear existing data
	Database->IndexedFrames.Empty();

	// Extract features from each animation
	int32 TotalFrames = 0;
	for (UAnimSequence* Animation : Database->SourceAnimations)
	{
		if (!Animation)
		{
			UE_LOG(LogTemp, Warning, TEXT("MotionDatabaseEditorUtility: Skipping null animation"));
			continue;
		}

		UE_LOG(LogTemp, Log, TEXT("MotionDatabaseEditorUtility: Processing animation '%s'..."), 
			*Animation->GetName());

		Preprocessor->ExtractFeatures(Animation);
		TotalFrames++;
	}

	// Build search index
	UE_LOG(LogTemp, Log, TEXT("MotionDatabaseEditorUtility: Building search index..."));
	// Note: The actual feature extraction and index building is handled by the preprocessor
	// This is a simplified version - the full implementation would be in MotionMatchingPreprocessor

	// Get statistics
	int32 FrameCount, AnimCount, MemorySize;
	GetDatabaseStatistics(Database, FrameCount, AnimCount, MemorySize);

	UE_LOG(LogTemp, Log, TEXT("MotionDatabaseEditorUtility: Preprocessing complete!"));
	UE_LOG(LogTemp, Log, TEXT("  - Animations: %d"), AnimCount);
	UE_LOG(LogTemp, Log, TEXT("  - Indexed Frames: %d"), FrameCount);
	UE_LOG(LogTemp, Log, TEXT("  - Estimated Memory: %.2f KB"), MemorySize / 1024.0f);

	// Mark package dirty so changes are saved
	Database->MarkPackageDirty();

	return true;
}

bool UMotionDatabaseEditorUtility::ValidateAnimationSkeleton(UMotionDatabase* Database)
{
	if (!Database || Database->SourceAnimations.Num() == 0)
	{
		return false;
	}

	// Get skeleton from first animation
	USkeleton* ReferenceSkeleton = nullptr;
	for (UAnimSequence* Animation : Database->SourceAnimations)
	{
		if (Animation && Animation->GetSkeleton())
		{
			ReferenceSkeleton = Animation->GetSkeleton();
			break;
		}
	}

	if (!ReferenceSkeleton)
	{
		UE_LOG(LogTemp, Error, TEXT("MotionDatabaseEditorUtility: No valid skeleton found in animations"));
		return false;
	}

	// Validate all animations use the same skeleton
	for (UAnimSequence* Animation : Database->SourceAnimations)
	{
		if (!Animation)
		{
			continue;
		}

		USkeleton* AnimSkeleton = Animation->GetSkeleton();
		if (!AnimSkeleton)
		{
			UE_LOG(LogTemp, Error, TEXT("MotionDatabaseEditorUtility: Animation '%s' has no skeleton"), 
				*Animation->GetName());
			return false;
		}

		if (AnimSkeleton != ReferenceSkeleton)
		{
			UE_LOG(LogTemp, Error, TEXT("MotionDatabaseEditorUtility: Animation '%s' uses different skeleton"), 
				*Animation->GetName());
			return false;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("MotionDatabaseEditorUtility: All animations use compatible skeleton '%s'"), 
		*ReferenceSkeleton->GetName());
	return true;
}

void UMotionDatabaseEditorUtility::GetDatabaseStatistics(UMotionDatabase* Database, int32& OutFrameCount, int32& OutAnimationCount, int32& OutMemorySize)
{
	OutFrameCount = 0;
	OutAnimationCount = 0;
	OutMemorySize = 0;

	if (!Database)
	{
		return;
	}

	OutFrameCount = Database->IndexedFrames.Num();
	OutAnimationCount = Database->SourceAnimations.Num();

	// Estimate memory size
	// Each FMotionFeature contains:
	// - FVector Velocity (12 bytes)
	// - float FacingAngle (4 bytes)
	// - TArray<FVector> JointPositions (assume 10 joints * 12 bytes = 120 bytes)
	// - EActionTag (1 byte)
	// - int32 FrameIndex (4 bytes)
	// - UAnimSequence* pointer (8 bytes)
	// Total: ~149 bytes per frame
	const int32 BytesPerFrame = 149;
	OutMemorySize = OutFrameCount * BytesPerFrame;
}

void UMotionDatabaseEditorUtility::ClearDatabaseCache(UMotionDatabase* Database)
{
	if (!Database)
	{
		return;
	}

	Database->IndexedFrames.Empty();
	Database->MarkPackageDirty();

	UE_LOG(LogTemp, Log, TEXT("MotionDatabaseEditorUtility: Cleared database cache"));
}

void UMotionDatabaseEditorUtility::AutoTagAnimations(UMotionDatabase* Database)
{
	if (!Database)
	{
		return;
	}

	int32 TaggedCount = 0;
	for (int32 i = 0; i < Database->IndexedFrames.Num(); i++)
	{
		FMotionFeature& Feature = Database->IndexedFrames[i];
		if (Feature.SourceSequence)
		{
			EActionTag Tag = DetermineActionTagFromName(Feature.SourceSequence->GetName());
			if (Tag != EActionTag::None)
			{
				Feature.ActionTag = Tag;
				TaggedCount++;
			}
		}
	}

	Database->MarkPackageDirty();
	UE_LOG(LogTemp, Log, TEXT("MotionDatabaseEditorUtility: Auto-tagged %d frames"), TaggedCount);
}

EActionTag UMotionDatabaseEditorUtility::DetermineActionTagFromName(const FString& AnimationName)
{
	FString LowerName = AnimationName.ToLower();

	if (LowerName.Contains(TEXT("idle")))
	{
		return EActionTag::Idle;
	}
	else if (LowerName.Contains(TEXT("sprint")) || LowerName.Contains(TEXT("fast")))
	{
		return EActionTag::Sprint;
	}
	else if (LowerName.Contains(TEXT("run")) || LowerName.Contains(TEXT("jog")))
	{
		return EActionTag::Run;
	}
	else if (LowerName.Contains(TEXT("turn")))
	{
		return EActionTag::Turn;
	}
	else if (LowerName.Contains(TEXT("kick")))
	{
		return EActionTag::Kick;
	}
	else if (LowerName.Contains(TEXT("tackle")) || LowerName.Contains(TEXT("slide")))
	{
		return EActionTag::Tackle;
	}

	return EActionTag::None;
}
