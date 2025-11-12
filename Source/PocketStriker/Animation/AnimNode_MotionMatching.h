// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNodeBase.h"
#include "AnimNode_MotionMatching.generated.h"

class UMotionDatabase;
struct FMotionFeature;
struct FMotionSearchResult;

/**
 * Animation graph node for motion matching
 * Integrates motion matching system into animation blueprint
 */
USTRUCT(BlueprintInternalUseOnly)
struct POCKETSTRIKER_API FAnimNode_MotionMatching : public FAnimNode_Base
{
	GENERATED_BODY()

public:
	FAnimNode_MotionMatching();

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// End of FAnimNode_Base interface

	// Motion database to search
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching", meta = (PinShownByDefault))
	UMotionDatabase* MotionDatabase;

	// Enable async search
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
	bool bUseAsyncSearch;

	// Blend time between poses
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
	float BlendTime;

private:
	// Build query feature from current state
	FMotionFeature BuildQueryFeature(const FAnimationUpdateContext& Context) const;

	// Find best matching frame
	FMotionSearchResult FindBestMatch(const FMotionFeature& Query);

	// Blend between current and target pose
	void BlendPoses(FPoseContext& Output, const FMotionSearchResult& Target, float Alpha);

	// Current animation state
	FMotionSearchResult CurrentMatch;
	float CurrentBlendAlpha;
	float TimeSinceLastUpdate;

	// Cached pose data
	FCompactPose PreviousPose;
	FBlendedCurve PreviousCurve;
	UE::Anim::FStackAttributeContainer PreviousAttributes;
};
