// Copyright Epic Games, Inc. All Rights Reserved.

#include "MotionMatcher.h"
#include "MotionDatabase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Async/Async.h"

// Async task implementation
void FMotionMatchingSearchTask::DoWork()
{
	double StartTime = FPlatformTime::Seconds();

	BestScore = FLT_MAX;
	BestIndex = 0;
	TopScores.Reset();

	// Early termination threshold - if we find a match this good, stop searching
	const float EarlyTerminationThreshold = 50.0f;

	// Spatial hashing optimization: Group candidates by velocity magnitude
	// This allows us to search nearby velocity space first
	TMap<int32, TArray<int32>> VelocityBuckets;
	const float BucketSize = 100.0f; // cm/s per bucket
	
	// Build spatial hash
	for (int32 i = 0; i < Database.Num(); ++i)
	{
		int32 BucketIndex = FMath::FloorToInt(Database[i].Velocity.Size() / BucketSize);
		VelocityBuckets.FindOrAdd(BucketIndex).Add(i);
	}

	// Determine query bucket
	int32 QueryBucket = FMath::FloorToInt(Query.Velocity.Size() / BucketSize);

	// Search order: query bucket first, then expand outward
	TArray<int32> SearchOrder;
	SearchOrder.Append(VelocityBuckets.FindOrAdd(QueryBucket));
	
	// Add adjacent buckets
	for (int32 Offset = 1; Offset <= 2; ++Offset)
	{
		SearchOrder.Append(VelocityBuckets.FindOrAdd(QueryBucket - Offset));
		SearchOrder.Append(VelocityBuckets.FindOrAdd(QueryBucket + Offset));
	}

	// If we haven't covered all candidates, add remaining
	if (SearchOrder.Num() < Database.Num())
	{
		for (int32 i = 0; i < Database.Num(); ++i)
		{
			if (!SearchOrder.Contains(i))
			{
				SearchOrder.Add(i);
			}
		}
	}

	// Search through candidates in optimized order
	for (int32 i : SearchOrder)
	{
		if (i < 0 || i >= Database.Num())
		{
			continue;
		}

		const FMotionFeature& Candidate = Database[i];
		
		// Calculate distance metric with SIMD-friendly operations
		float Score = 0.0f;

		// Velocity difference (weighted heavily)
		FVector VelocityDiff = Query.Velocity - Candidate.Velocity;
		Score += VelocityDiff.SizeSquared() * 2.0f;

		// Early rejection: if velocity alone is too different, skip detailed comparison
		if (Score > BestScore * 2.0f)
		{
			continue;
		}

		// Facing angle difference
		float AngleDiff = FMath::Abs(Query.FacingAngle - Candidate.FacingAngle);
		if (AngleDiff > 180.0f)
		{
			AngleDiff = 360.0f - AngleDiff;
		}
		Score += AngleDiff * AngleDiff * 0.5f;

		// Joint position differences (SIMD-friendly loop)
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

		// Track top candidates for debug visualization
		FCandidateScore CandidateScore;
		CandidateScore.Index = i;
		CandidateScore.Score = Score;
		
		if (TopScores.Num() < 5)
		{
			TopScores.Add(CandidateScore);
			TopScores.Sort();
		}
		else if (Score < TopScores.Last().Score)
		{
			TopScores.Last() = CandidateScore;
			TopScores.Sort();
		}

		if (Score < BestScore)
		{
			BestScore = Score;
			BestIndex = i;

			// Early termination: if we found a very good match, stop searching
			if (BestScore < EarlyTerminationThreshold)
			{
				break;
			}
		}
	}

	double EndTime = FPlatformTime::Seconds();
	SearchTime = (EndTime - StartTime) * 1000.0; // Convert to milliseconds
}

FMotionSearchResult FMotionMatchingSearchTask::GetResult() const
{
	FMotionSearchResult Result;
	
	if (BestIndex >= 0 && BestIndex < Database.Num())
	{
		Result.BestMatch = Database[BestIndex];
		Result.MatchScore = BestScore;
		Result.SearchTime = static_cast<float>(SearchTime);
	}
	
	return Result;
}

TArray<FMotionSearchResult> FMotionMatchingSearchTask::GetTopResults(int32 Count) const
{
	TArray<FMotionSearchResult> Results;
	
	for (int32 i = 0; i < FMath::Min(Count, TopScores.Num()); ++i)
	{
		const FCandidateScore& CandScore = TopScores[i];
		if (CandScore.Index >= 0 && CandScore.Index < Database.Num())
		{
			FMotionSearchResult Result;
			Result.BestMatch = Database[CandScore.Index];
			Result.MatchScore = CandScore.Score;
			Result.SearchTime = static_cast<float>(SearchTime);
			Results.Add(Result);
		}
	}
	
	return Results;
}

UMotionMatcher::UMotionMatcher()
	: BlendAlpha(0.0f)
	, bUseAsyncSearch(true)
	, bAsyncSearchPending(false)
	, PerformanceThreshold(2.5f)
	, FallbackBlendTime(0.3f)
	, bUsingFallback(false)
	, FallbackTransitionAlpha(0.0f)
	, BlendspaceInput(FVector2D::ZeroVector)
	, SearchTimeIndex(0)
{
	RecentSearchTimes.SetNum(MaxSearchTimeSamples);
	for (int32 i = 0; i < MaxSearchTimeSamples; ++i)
	{
		RecentSearchTimes[i] = 0.0f;
	}
}

void UMotionMatcher::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Check if we should use fallback system
	bool bShouldUseFallback = ShouldUseFallback();
	
	if (bShouldUseFallback)
	{
		bUsingFallback = true;
		UpdateBlendspace(DeltaSeconds);
		return;
	}

	bUsingFallback = false;

	if (!MotionDatabase || MotionDatabase->IndexedFrames.Num() == 0)
	{
		return;
	}

	// Build query feature from current character state
	FMotionFeature QueryFeature = BuildQueryFeature();
	LastQueryFeature = QueryFeature; // Store for debug display

	FMotionSearchResult SearchResult;

	if (bUseAsyncSearch)
	{
		// Check if previous async search is complete
		if (IsAsyncSearchComplete())
		{
			SearchResult = GetAsyncSearchResult();
			CurrentSearchResult = SearchResult;
			
			// Track search time for performance monitoring
			RecentSearchTimes[SearchTimeIndex] = SearchResult.SearchTime;
			SearchTimeIndex = (SearchTimeIndex + 1) % MaxSearchTimeSamples;
		}
		else
		{
			// Use previous result while waiting
			SearchResult = CurrentSearchResult;
		}

		// Start new async search for next frame
		AsyncSearchMotionDatabase(QueryFeature);
	}
	else
	{
		// Synchronous search
		SearchResult = FindBestMatch(QueryFeature);
		CurrentSearchResult = SearchResult;
		
		// Track search time for performance monitoring
		RecentSearchTimes[SearchTimeIndex] = SearchResult.SearchTime;
		SearchTimeIndex = (SearchTimeIndex + 1) % MaxSearchTimeSamples;
	}

	// Blend to the target frame
	BlendToTarget(SearchResult, DeltaSeconds);
}

FMotionFeature UMotionMatcher::BuildQueryFeature() const
{
	FMotionFeature Query;

	// Get character velocity and facing
	ACharacter* Character = Cast<ACharacter>(TryGetPawnOwner());
	if (Character)
	{
		UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
		if (Movement)
		{
			Query.Velocity = Movement->Velocity;
		}

		// Get facing angle from character rotation
		FRotator Rotation = Character->GetActorRotation();
		Query.FacingAngle = Rotation.Yaw;
	}

	// Extract current pose joint positions
	// In production, this would sample actual bone transforms from the skeleton
	Query.JointPositions.Add(FVector(0.0f, 0.0f, 100.0f)); // Hips
	Query.JointPositions.Add(FVector(0.0f, -20.0f, 0.0f)); // Left foot
	Query.JointPositions.Add(FVector(0.0f, 20.0f, 0.0f));  // Right foot
	Query.JointPositions.Add(FVector(-50.0f, -30.0f, 100.0f)); // Left hand
	Query.JointPositions.Add(FVector(-50.0f, 30.0f, 100.0f));  // Right hand

	Query.ActionTag = EActionTag::Run; // Would be determined by game state

	return Query;
}

FMotionSearchResult UMotionMatcher::FindBestMatch(const FMotionFeature& Query)
{
	FMotionSearchResult Result;
	
	if (!MotionDatabase || MotionDatabase->IndexedFrames.Num() == 0)
	{
		Result.MatchScore = FLT_MAX;
		Result.SearchTime = 0.0f;
		return Result;
	}

	// Start timing the search
	double StartTime = FPlatformTime::Seconds();

	float BestScore = FLT_MAX;
	int32 BestIndex = 0;

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
	TArray<FCandidateScore> LocalTopScores;

	// Early termination threshold - if we find a match this good, stop searching
	const float EarlyTerminationThreshold = 50.0f;

	// Spatial hashing optimization: Group candidates by velocity magnitude
	TMap<int32, TArray<int32>> VelocityBuckets;
	const float BucketSize = 100.0f; // cm/s per bucket
	
	// Build spatial hash
	for (int32 i = 0; i < MotionDatabase->IndexedFrames.Num(); ++i)
	{
		int32 BucketIndex = FMath::FloorToInt(MotionDatabase->IndexedFrames[i].Velocity.Size() / BucketSize);
		VelocityBuckets.FindOrAdd(BucketIndex).Add(i);
	}

	// Determine query bucket
	int32 QueryBucket = FMath::FloorToInt(Query.Velocity.Size() / BucketSize);

	// Search order: query bucket first, then expand outward
	TArray<int32> SearchOrder;
	SearchOrder.Append(VelocityBuckets.FindOrAdd(QueryBucket));
	
	// Add adjacent buckets
	for (int32 Offset = 1; Offset <= 2; ++Offset)
	{
		SearchOrder.Append(VelocityBuckets.FindOrAdd(QueryBucket - Offset));
		SearchOrder.Append(VelocityBuckets.FindOrAdd(QueryBucket + Offset));
	}

	// If we haven't covered all candidates, add remaining
	if (SearchOrder.Num() < MotionDatabase->IndexedFrames.Num())
	{
		for (int32 i = 0; i < MotionDatabase->IndexedFrames.Num(); ++i)
		{
			if (!SearchOrder.Contains(i))
			{
				SearchOrder.Add(i);
			}
		}
	}

	// Search through candidates in optimized order
	for (int32 i : SearchOrder)
	{
		if (i < 0 || i >= MotionDatabase->IndexedFrames.Num())
		{
			continue;
		}

		const FMotionFeature& Candidate = MotionDatabase->IndexedFrames[i];
		
		// Calculate distance metric with SIMD-friendly operations
		float Score = 0.0f;

		// Velocity difference (weighted heavily)
		FVector VelocityDiff = Query.Velocity - Candidate.Velocity;
		Score += VelocityDiff.SizeSquared() * 2.0f;

		// Early rejection: if velocity alone is too different, skip detailed comparison
		if (Score > BestScore * 2.0f)
		{
			continue;
		}

		// Facing angle difference
		float AngleDiff = FMath::Abs(Query.FacingAngle - Candidate.FacingAngle);
		if (AngleDiff > 180.0f)
		{
			AngleDiff = 360.0f - AngleDiff;
		}
		Score += AngleDiff * AngleDiff * 0.5f;

		// Joint position differences (SIMD-friendly loop)
		int32 NumJoints = FMath::Min(Query.JointPositions.Num(), Candidate.JointPositions.Num());
		for (int32 j = 0; j < NumJoints; ++j)
		{
			FVector JointDiff = Query.JointPositions[j] - Candidate.JointPositions[j];
			Score += JointDiff.SizeSquared() * 0.1f;
		}

		// Action tag match bonus
		if (Query.ActionTag == Candidate.ActionTag)
		{
			Score *= 0.5f; // Prefer matching action tags
		}

		// Track top candidates for debug visualization
		FCandidateScore CandidateScore;
		CandidateScore.Index = i;
		CandidateScore.Score = Score;
		
		if (LocalTopScores.Num() < MaxTopCandidates)
		{
			LocalTopScores.Add(CandidateScore);
			LocalTopScores.Sort();
		}
		else if (Score < LocalTopScores.Last().Score)
		{
			LocalTopScores.Last() = CandidateScore;
			LocalTopScores.Sort();
		}

		if (Score < BestScore)
		{
			BestScore = Score;
			BestIndex = i;

			// Early termination: if we found a very good match, stop searching
			if (BestScore < EarlyTerminationThreshold)
			{
				break;
			}
		}
	}

	// Record search time
	double EndTime = FPlatformTime::Seconds();
	Result.SearchTime = static_cast<float>((EndTime - StartTime) * 1000.0f); // Convert to milliseconds

	Result.BestMatch = MotionDatabase->IndexedFrames[BestIndex];
	Result.MatchScore = BestScore;

	// Store top candidates for debug display
	TopCandidates.Reset();
	for (const FCandidateScore& CandScore : LocalTopScores)
	{
		if (CandScore.Index >= 0 && CandScore.Index < MotionDatabase->IndexedFrames.Num())
		{
			FMotionSearchResult CandResult;
			CandResult.BestMatch = MotionDatabase->IndexedFrames[CandScore.Index];
			CandResult.MatchScore = CandScore.Score;
			CandResult.SearchTime = Result.SearchTime;
			TopCandidates.Add(CandResult);
		}
	}

	// Log if search time exceeds target
	if (Result.SearchTime > 2.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Motion matching search exceeded 2ms target: %.2fms"), Result.SearchTime);
	}

	return Result;
}

void UMotionMatcher::BlendToTarget(const FMotionSearchResult& Target, float DeltaTime)
{
	// Temporal smoothing to prevent animation popping
	const float BlendSpeed = 5.0f; // Blend over ~200ms
	
	BlendAlpha = FMath::FInterpTo(BlendAlpha, 1.0f, DeltaTime, BlendSpeed);

	// In production, this would blend between current pose and target pose
	// For now, we just track the blend alpha for debugging
	
	if (BlendAlpha >= 0.99f)
	{
		BlendAlpha = 0.0f; // Reset for next transition
	}
}

void UMotionMatcher::AsyncSearchMotionDatabase(const FMotionFeature& Query)
{
	if (!MotionDatabase || MotionDatabase->IndexedFrames.Num() == 0)
	{
		return;
	}

	// Don't start new search if one is already running
	if (bAsyncSearchPending && AsyncSearchTask.IsValid() && !AsyncSearchTask->IsDone())
	{
		return;
	}

	// Create and start async task
	AsyncSearchTask = MakeShared<FAsyncTask<FMotionMatchingSearchTask>>(
		Query, 
		MotionDatabase->IndexedFrames
	);
	
	AsyncSearchTask->StartBackgroundTask();
	bAsyncSearchPending = true;
}

bool UMotionMatcher::IsAsyncSearchComplete() const
{
	if (!bAsyncSearchPending || !AsyncSearchTask.IsValid())
	{
		return false;
	}

	return AsyncSearchTask->IsDone();
}

FMotionSearchResult UMotionMatcher::GetAsyncSearchResult()
{
	if (!AsyncSearchTask.IsValid() || !AsyncSearchTask->IsDone())
	{
		return CurrentSearchResult;
	}

	FMotionSearchResult Result = AsyncSearchTask->GetTask().GetResult();
	
	// Get top candidates for debug display
	TopCandidates = AsyncSearchTask->GetTask().GetTopResults(MaxTopCandidates);
	
	bAsyncSearchPending = false;

	// Log if search time exceeds target
	if (Result.SearchTime > 2.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Async motion matching search exceeded 2ms target: %.2fms"), Result.SearchTime);
	}

	return Result;
}

bool UMotionMatcher::ShouldUseFallback() const
{
	// Check if motion matching is performing poorly
	
	if (!MotionDatabase || MotionDatabase->IndexedFrames.Num() == 0)
	{
		return true;
	}

	// Calculate average search time from recent samples
	float AverageSearchTime = 0.0f;
	int32 ValidSamples = 0;
	
	for (float SearchTime : RecentSearchTimes)
	{
		if (SearchTime > 0.0f)
		{
			AverageSearchTime += SearchTime;
			ValidSamples++;
		}
	}

	if (ValidSamples > 0)
	{
		AverageSearchTime /= ValidSamples;
		
		// Use fallback if average search time exceeds threshold
		if (AverageSearchTime > PerformanceThreshold)
		{
			return true;
		}
	}

	return false;
}

void UMotionMatcher::UpdateBlendspace(float DeltaTime)
{
	// Update blendspace input based on character velocity
	ACharacter* Character = Cast<ACharacter>(TryGetPawnOwner());
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	if (!Movement)
	{
		return;
	}

	// Convert velocity to blendspace coordinates
	FVector Velocity = Movement->Velocity;
	FVector LocalVelocity = Character->GetActorRotation().UnrotateVector(Velocity);
	
	// Normalize to blendspace range (-1 to 1)
	float MaxSpeed = Movement->GetMaxSpeed();
	if (MaxSpeed > 0.0f)
	{
		BlendspaceInput.X = FMath::Clamp(LocalVelocity.X / MaxSpeed, -1.0f, 1.0f);
		BlendspaceInput.Y = FMath::Clamp(LocalVelocity.Y / MaxSpeed, -1.0f, 1.0f);
	}

	// Smooth transition to fallback mode
	if (bUsingFallback)
	{
		FallbackTransitionAlpha = FMath::FInterpTo(FallbackTransitionAlpha, 1.0f, DeltaTime, 1.0f / FallbackBlendTime);
	}
	else
	{
		FallbackTransitionAlpha = FMath::FInterpTo(FallbackTransitionAlpha, 0.0f, DeltaTime, 1.0f / FallbackBlendTime);
	}
}

float UMotionMatcher::GetAverageSearchTime() const
{
	float AverageSearchTime = 0.0f;
	int32 ValidSamples = 0;
	
	for (float SearchTime : RecentSearchTimes)
	{
		if (SearchTime > 0.0f)
		{
			AverageSearchTime += SearchTime;
			ValidSamples++;
		}
	}

	if (ValidSamples > 0)
	{
		AverageSearchTime /= ValidSamples;
	}

	return AverageSearchTime;
}
