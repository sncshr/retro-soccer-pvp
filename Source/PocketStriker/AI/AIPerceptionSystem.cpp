// Copyright Epic Games, Inc. All Rights Reserved.

#include "AIPerceptionSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UAIPerceptionSystem::UAIPerceptionSystem()
	: GridCellSize(500.0f) // 5 meter cells
	, LastGridBuildTime(0.0)
	, GridRebuildInterval(1.0f) // Rebuild grid every second
	, CacheLifetime(0.2f) // Cache queries for 200ms
	, CacheHitCount(0)
	, CacheMissCount(0)
	, QueryTimeIndex(0)
{
	RecentQueryTimes.SetNum(MaxQueryTimeSamples);
	for (int32 i = 0; i < MaxQueryTimeSamples; ++i)
	{
		RecentQueryTimes[i] = 0.0f;
	}
}

bool UAIPerceptionSystem::DetectBall(const FVector& AILocation, float Radius, FVector& OutBallLocation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Find ball actor by tag
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(World, FName("Ball"), FoundActors);

	if (FoundActors.Num() == 0)
	{
		return false;
	}

	AActor* Ball = FoundActors[0];
	FVector BallLocation = Ball->GetActorLocation();
	float DistanceToBall = FVector::Dist(AILocation, BallLocation);

	// Check if ball is within perception radius
	if (DistanceToBall > Radius)
	{
		return false;
	}

	// Check line of sight
	if (!HasLineOfSight(AILocation, BallLocation))
	{
		return false;
	}

	OutBallLocation = BallLocation;
	return true;
}

void UAIPerceptionSystem::BuildProximityGrid()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Check if we need to rebuild (throttle grid rebuilds)
	double CurrentTime = World->GetTimeSeconds();
	if (CurrentTime - LastGridBuildTime < GridRebuildInterval)
	{
		return; // Grid is still fresh
	}

	double StartTime = FPlatformTime::Seconds();

	ProximityGrid.Empty();

	// Get all actors with AI tag
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsWithTag(World, FName("AI"), AllActors);

	// Add player actors
	TArray<AActor*> PlayerActors;
	UGameplayStatics::GetAllActorsWithTag(World, FName("Player"), PlayerActors);
	AllActors.Append(PlayerActors);

	// Insert actors into grid cells
	for (AActor* Actor : AllActors)
	{
		if (!Actor)
		{
			continue;
		}

		FVector Location = Actor->GetActorLocation();
		FIntVector CellIndex(
			FMath::FloorToInt(Location.X / GridCellSize),
			FMath::FloorToInt(Location.Y / GridCellSize),
			FMath::FloorToInt(Location.Z / GridCellSize)
		);

		ProximityGrid.FindOrAdd(CellIndex).Add(Actor);
	}

	LastGridBuildTime = CurrentTime;

	// Track build time for performance monitoring
	double EndTime = FPlatformTime::Seconds();
	float BuildTime = static_cast<float>((EndTime - StartTime) * 1000.0f);
	
	// Log if build time is excessive
	if (BuildTime > 1.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Proximity grid build took %.2fms"), BuildTime);
	}
}

TArray<AActor*> UAIPerceptionSystem::QueryNearbyActors(const FVector& Location, float Radius)
{
	double StartTime = FPlatformTime::Seconds();

	// Check cache first
	int32 CacheKey = GetQueryCacheKey(Location, Radius);
	if (FCachedProximityQuery* CachedQuery = QueryCache.Find(CacheKey))
	{
		if (IsCacheValid(*CachedQuery, Location, Radius))
		{
			CacheHitCount++;
			return CachedQuery->Results;
		}
	}

	CacheMissCount++;

	// Rebuild grid if needed
	BuildProximityGrid();

	TArray<AActor*> NearbyActors;

	// Calculate grid cell range to check
	int32 CellRadius = FMath::CeilToInt(Radius / GridCellSize);
	FIntVector CenterCell(
		FMath::FloorToInt(Location.X / GridCellSize),
		FMath::FloorToInt(Location.Y / GridCellSize),
		FMath::FloorToInt(Location.Z / GridCellSize)
	);

	// Check all cells within range
	for (int32 X = -CellRadius; X <= CellRadius; ++X)
	{
		for (int32 Y = -CellRadius; Y <= CellRadius; ++Y)
		{
			for (int32 Z = -CellRadius; Z <= CellRadius; ++Z)
			{
				FIntVector CellIndex = CenterCell + FIntVector(X, Y, Z);
				
				if (TArray<AActor*>* CellActors = ProximityGrid.Find(CellIndex))
				{
					for (AActor* Actor : *CellActors)
					{
						if (!Actor)
						{
							continue;
						}

						float Distance = FVector::Dist(Location, Actor->GetActorLocation());
						if (Distance <= Radius)
						{
							NearbyActors.AddUnique(Actor);
						}
					}
				}
			}
		}
	}

	// Cache the result
	FCachedProximityQuery NewCache;
	NewCache.Location = Location;
	NewCache.Radius = Radius;
	NewCache.Results = NearbyActors;
	NewCache.Timestamp = FPlatformTime::Seconds();
	QueryCache.Add(CacheKey, NewCache);

	// Track query time for performance monitoring
	double EndTime = FPlatformTime::Seconds();
	float QueryTime = static_cast<float>((EndTime - StartTime) * 1000.0f);
	RecentQueryTimes[QueryTimeIndex] = QueryTime;
	QueryTimeIndex = (QueryTimeIndex + 1) % MaxQueryTimeSamples;

	return NearbyActors;
}

bool UAIPerceptionSystem::HasLineOfSight(const FVector& From, const FVector& To) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Perform line trace
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.bReturnPhysicalMaterial = false;

	// Trace from eye height to target
	FVector Start = From + FVector(0, 0, 80.0f); // Approximate eye height
	FVector End = To;

	bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	// If no hit, we have line of sight
	// If hit, check if we hit the target (within small tolerance)
	if (!bHit)
	{
		return true;
	}

	float DistanceToTarget = FVector::Dist(Start, End);
	float DistanceToHit = FVector::Dist(Start, HitResult.Location);

	// If we hit very close to the target, consider it visible
	return (DistanceToTarget - DistanceToHit) < 10.0f;
}

void UAIPerceptionSystem::InvalidateCache()
{
	QueryCache.Empty();
	CacheHitCount = 0;
	CacheMissCount = 0;
}

int32 UAIPerceptionSystem::GetQueryCacheKey(const FVector& Location, float Radius) const
{
	// Simple hash based on quantized location and radius
	int32 X = FMath::FloorToInt(Location.X / 100.0f);
	int32 Y = FMath::FloorToInt(Location.Y / 100.0f);
	int32 Z = FMath::FloorToInt(Location.Z / 100.0f);
	int32 R = FMath::FloorToInt(Radius / 100.0f);
	
	return HashCombine(HashCombine(HashCombine(X, Y), Z), R);
}

bool UAIPerceptionSystem::IsCacheValid(const FCachedProximityQuery& CachedQuery, const FVector& Location, float Radius) const
{
	// Check if cache entry has expired
	double CurrentTime = FPlatformTime::Seconds();
	if (CurrentTime - CachedQuery.Timestamp > CacheLifetime)
	{
		return false;
	}

	// Check if query parameters are similar enough
	float LocationDiff = FVector::Dist(CachedQuery.Location, Location);
	float RadiusDiff = FMath::Abs(CachedQuery.Radius - Radius);

	// Cache is valid if query is within 50cm and radius is within 50cm
	return (LocationDiff < 50.0f && RadiusDiff < 50.0f);
}

float UAIPerceptionSystem::GetAverageQueryTime() const
{
	float AverageQueryTime = 0.0f;
	int32 ValidSamples = 0;
	
	for (float QueryTime : RecentQueryTimes)
	{
		if (QueryTime > 0.0f)
		{
			AverageQueryTime += QueryTime;
			ValidSamples++;
		}
	}

	if (ValidSamples > 0)
	{
		AverageQueryTime /= ValidSamples;
	}

	return AverageQueryTime;
}
