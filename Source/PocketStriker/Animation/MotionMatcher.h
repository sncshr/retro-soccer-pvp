// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MotionMatcher.generated.h"

class UMotionDatabase;
struct FMotionFeature;
struct FMotionSearchResult;

/**
 * Async task for motion matching search
 * Runs on worker thread to avoid blocking game thread
 */
class FMotionMatchingSearchTask : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FMotionMatchingSearchTask>;

public:
	FMotionMatchingSearchTask(const FMotionFeature& InQuery, const TArray<FMotionFeature>& InDatabase)
		: Query(InQuery)
		, Database(InDatabase)
		, BestScore(FLT_MAX)
		, BestIndex(0)
	{
	}

	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FMotionMatchingSearchTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	FMotionSearchResult GetResult() const;
	TArray<FMotionSearchResult> GetTopResults(int32 Count) const;

private:
	FMotionFeature Query;
	TArray<FMotionFeature> Database;
	float BestScore;
	int32 BestIndex;
	double SearchTime;

	// Track top candidates for debug visualization
	struct FCandidateScore
	{
		int32 Index;
		float Score;

		bool operator<(const FCandidateScore& Other) const
		{
			return Score < Other.Score;
		}
	};
	TArray<FCandidateScore> TopScores;
};

/**
 * Motion matching animation instance
 * Selects best animation frame based on current movement state
 */
UCLASS()
class POCKETSTRIKER_API UMotionMatcher : public UAnimInstance
{
	GENERATED_BODY()

public:
	UMotionMatcher();

	// Query and search
	FMotionFeature BuildQueryFeature() const;
	FMotionSearchResult FindBestMatch(const FMotionFeature& Query);
	
	// Blending and smoothing
	void BlendToTarget(const FMotionSearchResult& Target, float DeltaTime);
	
	// Performance optimization
	void AsyncSearchMotionDatabase(const FMotionFeature& Query);
	bool IsAsyncSearchComplete() const;
	FMotionSearchResult GetAsyncSearchResult();
	
	// Fallback system
	bool ShouldUseFallback() const;
	void UpdateBlendspace(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
	UMotionDatabase* MotionDatabase;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
	bool bUseAsyncSearch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fallback")
	float PerformanceThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fallback")
	float FallbackBlendTime;

	// Debug info accessors
	UFUNCTION(BlueprintCallable, Category = "Motion Matching Debug")
	FMotionSearchResult GetCurrentSearchResult() const { return CurrentSearchResult; }

	UFUNCTION(BlueprintCallable, Category = "Motion Matching Debug")
	float GetAverageSearchTime() const;

	UFUNCTION(BlueprintCallable, Category = "Motion Matching Debug")
	FMotionFeature GetLastQueryFeature() const { return LastQueryFeature; }

	UFUNCTION(BlueprintCallable, Category = "Motion Matching Debug")
	TArray<float> GetRecentSearchTimes() const { return RecentSearchTimes; }

	UFUNCTION(BlueprintCallable, Category = "Motion Matching Debug")
	TArray<FMotionSearchResult> GetTopCandidates() const { return TopCandidates; }

protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	float BlendAlpha;
	FMotionSearchResult CurrentSearchResult;
	FMotionSearchResult PendingSearchResult;
	FMotionFeature LastQueryFeature;
	
	// Async task management
	TSharedPtr<FAsyncTask<FMotionMatchingSearchTask>> AsyncSearchTask;
	bool bAsyncSearchPending;

	// Fallback system
	bool bUsingFallback;
	float FallbackTransitionAlpha;
	FVector2D BlendspaceInput;
	
	// Performance tracking
	TArray<float> RecentSearchTimes;
	int32 SearchTimeIndex;
	static constexpr int32 MaxSearchTimeSamples = 30;

	// Debug: Top candidate matches for visualization
	TArray<FMotionSearchResult> TopCandidates;
	static constexpr int32 MaxTopCandidates = 5;
};
