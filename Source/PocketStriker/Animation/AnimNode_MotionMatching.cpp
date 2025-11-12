// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimNode_MotionMatching.h"
#include "MotionDatabase.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimSequence.h"

FAnimNode_MotionMatching::FAnimNode_MotionMatching()
	: MotionDatabase(nullptr)
	, bUseAsyncSearch(true)
	, BlendTime(0.2f)
	, CurrentBlendAlpha(0.0f)
	, TimeSinceLastUpdate(0.0f)
{
}

void FAnimNode_MotionMatching::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	FAnimNode_Base::Initialize_AnyThread(Context);
	
	CurrentBlendAlpha = 0.0f;
	TimeSinceLastUpdate = 0.0f;
}

void FAnimNode_MotionMatching::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	FAnimNode_Base::CacheBones_AnyThread(Context);
}

void FAnimNode_MotionMatching::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	FAnimNode_Base::Update_AnyThread(Context);

	if (!MotionDatabase || MotionDatabase->IndexedFrames.Num() == 0)
	{
		return;
	}

	TimeSinceLastUpdate += Context.GetDeltaTime();

	// Build query from current state
	FMotionFeature QueryFeature = BuildQueryFeature(Context);

	// Find best matching frame
	FMotionSearchResult SearchResult = FindBestMatch(QueryFeature);

	// Update blend alpha
	if (BlendTime > 0.0f)
	{
		CurrentBlendAlpha = FMath::Clamp(TimeSinceLastUpdate / BlendTime, 0.0f, 1.0f);
	}
	else
	{
		CurrentBlendAlpha = 1.0f;
	}

	// Store result for evaluation
	CurrentMatch = SearchResult;
}

void FAnimNode_MotionMatching::Evaluate_AnyThread(FPoseContext& Output)
{
	if (!MotionDatabase || MotionDatabase->IndexedFrames.Num() == 0)
	{
		// Output reference pose if no database
		Output.Pose.ResetToRefPose();
		return;
	}

	// Blend to target pose
	BlendPoses(Output, CurrentMatch, CurrentBlendAlpha);
}

void FAnimNode_MotionMatching::GatherDebugData(FNodeDebugData& DebugData)
{
	FString DebugLine = DebugData.GetNodeName(this);
	
	if (CurrentMatch.BestMatch.SourceSequence)
	{
		DebugLine += FString::Printf(TEXT("\nSequence: %s"), *CurrentMatch.BestMatch.SourceSequence->GetName());
		DebugLine += FString::Printf(TEXT("\nFrame: %d"), CurrentMatch.BestMatch.FrameIndex);
		DebugLine += FString::Printf(TEXT("\nScore: %.2f"), CurrentMatch.MatchScore);
		DebugLine += FString::Printf(TEXT("\nSearch Time: %.2fms"), CurrentMatch.SearchTime);
	}
	
	DebugData.AddDebugItem(DebugLine);
}

FMotionFeature FAnimNode_MotionMatching::BuildQueryFeature(const FAnimationUpdateContext& Context) const
{
	FMotionFeature Query;

	// Get velocity from animation proxy
	const FAnimInstanceProxy* Proxy = Context.AnimInstanceProxy;
	if (Proxy)
	{
		Query.Velocity = Proxy->GetActorTransform().GetRotation().UnrotateVector(Proxy->GetVelocity());
	}

	// Get facing angle
	if (Proxy)
	{
		FRotator Rotation = Proxy->GetActorTransform().GetRotation().Rotator();
		Query.FacingAngle = Rotation.Yaw;
	}

	// Extract joint positions (simplified)
	Query.JointPositions.Add(FVector(0.0f, 0.0f, 100.0f)); // Hips
	Query.JointPositions.Add(FVector(0.0f, -20.0f, 0.0f)); // Left foot
	Query.JointPositions.Add(FVector(0.0f, 20.0f, 0.0f));  // Right foot
	Query.JointPositions.Add(FVector(-50.0f, -30.0f, 100.0f)); // Left hand
	Query.JointPositions.Add(FVector(-50.0f, 30.0f, 100.0f));  // Right hand

	Query.ActionTag = EActionTag::Run;

	return Query;
}

FMotionSearchResult FAnimNode_MotionMatching::FindBestMatch(const FMotionFeature& Query)
{
	FMotionSearchResult Result;
	
	if (!MotionDatabase || MotionDatabase->IndexedFrames.Num() == 0)
	{
		Result.MatchScore = FLT_MAX;
		Result.SearchTime = 0.0f;
		return Result;
	}

	double StartTime = FPlatformTime::Seconds();

	float BestScore = FLT_MAX;
	int32 BestIndex = 0;

	// Linear search through all frames
	for (int32 i = 0; i < MotionDatabase->IndexedFrames.Num(); ++i)
	{
		const FMotionFeature& Candidate = MotionDatabase->IndexedFrames[i];
		
		// Calculate distance metric
		float Score = 0.0f;

		// Velocity difference
		FVector VelocityDiff = Query.Velocity - Candidate.Velocity;
		Score += VelocityDiff.SizeSquared() * 2.0f;

		// Facing angle difference
		float AngleDiff = FMath::Abs(Query.FacingAngle - Candidate.FacingAngle);
		if (AngleDiff > 180.0f)
		{
			AngleDiff = 360.0f - AngleDiff;
		}
		Score += AngleDiff * AngleDiff * 0.5f;

		// Joint position differences
		int32 NumJoints = FMath::Min(Query.JointPositions.Num(), Candidate.JointPositions.Num());
		for (int32 j = 0; j < NumJoints; ++j)
		{
			FVector JointDiff = Query.JointPositions[j] - Candidate.JointPositions[j];
			Score += JointDiff.SizeSquared() * 0.1f;
		}

		// Action tag match bonus
		if (Query.ActionTag == Candidate.ActionTag)
		{
			Score *= 0.5f;
		}

		if (Score < BestScore)
		{
			BestScore = Score;
			BestIndex = i;
		}
	}

	double EndTime = FPlatformTime::Seconds();
	Result.SearchTime = static_cast<float>((EndTime - StartTime) * 1000.0);

	Result.BestMatch = MotionDatabase->IndexedFrames[BestIndex];
	Result.MatchScore = BestScore;

	return Result;
}

void FAnimNode_MotionMatching::BlendPoses(FPoseContext& Output, const FMotionSearchResult& Target, float Alpha)
{
	// For this prototype, we output a reference pose
	// In production, this would blend between the previous pose and target animation frame
	
	Output.Pose.ResetToRefPose();
	
	// Apply root motion from target animation if available
	if (Target.BestMatch.SourceSequence)
	{
		// Root motion would be extracted and applied here
		// For now, we just ensure the pose is valid
	}

	// Blend alpha is used for smooth transitions
	// In production, this would lerp between cached previous pose and target pose
}
