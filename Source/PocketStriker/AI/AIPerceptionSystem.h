// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AIPerceptionSystem.generated.h"

/**
 * Cached query result for proximity grid optimization
 */
USTRUCT()
struct FCachedProximityQuery
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	float Radius;

	UPROPERTY()
	TArray<AActor*> Results;

	UPROPERTY()
	double Timestamp;

	FCachedProximityQuery()
		: Location(FVector::ZeroVector)
		, Radius(0.0f)
		, Timestamp(0.0)
	{
	}
};

/**
 * AI perception system for ball and actor detection
 * Provides spatial queries and line-of-sight checks
 * Optimized with proximity grid caching and query throttling
 */
UCLASS()
class POCKETSTRIKER_API UAIPerceptionSystem : public UObject
{
	GENERATED_BODY()

public:
	UAIPerceptionSystem();

	// Ball tracking
	bool DetectBall(const FVector& AILocation, float Radius, FVector& OutBallLocation);
	
	// Spatial queries
	void BuildProximityGrid();
	TArray<AActor*> QueryNearbyActors(const FVector& Location, float Radius);
	
	// Line of sight
	bool HasLineOfSight(const FVector& From, const FVector& To) const;

	// Cache management
	void InvalidateCache();
	void SetCacheLifetime(float Seconds) { CacheLifetime = Seconds; }

	// Performance tracking
	UFUNCTION(BlueprintCallable, Category = "AI Performance")
	float GetAverageQueryTime() const;

	UFUNCTION(BlueprintCallable, Category = "AI Performance")
	int32 GetCacheHitCount() const { return CacheHitCount; }

	UFUNCTION(BlueprintCallable, Category = "AI Performance")
	int32 GetCacheMissCount() const { return CacheMissCount; }

protected:
	UPROPERTY()
	TMap<FIntVector, TArray<AActor*>> ProximityGrid;

	float GridCellSize;
	double LastGridBuildTime;
	float GridRebuildInterval; // Seconds between grid rebuilds

	// Query caching
	TMap<int32, FCachedProximityQuery> QueryCache;
	float CacheLifetime; // Seconds before cache entry expires
	int32 CacheHitCount;
	int32 CacheMissCount;

	// Performance tracking
	TArray<float> RecentQueryTimes;
	int32 QueryTimeIndex;
	static constexpr int32 MaxQueryTimeSamples = 30;

	// Helper functions
	int32 GetQueryCacheKey(const FVector& Location, float Radius) const;
	bool IsCacheValid(const FCachedProximityQuery& CachedQuery, const FVector& Location, float Radius) const;
};
