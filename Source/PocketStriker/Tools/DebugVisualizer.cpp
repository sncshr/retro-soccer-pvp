// Copyright Epic Games, Inc. All Rights Reserved.

#include "DebugVisualizer.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"

UDebugVisualizer::UDebugVisualizer()
{
	bEnableVisualization = true;
	DebugLineDuration = 0.1f;
	DebugLineThickness = 2.0f;
}

void UDebugVisualizer::DrawAITargetLine(const FVector& From, const FVector& To)
{
	if (!bEnableVisualization)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Draw line from AI to target
	DrawDebugLine(World, From, To, FColor::Magenta, false, DebugLineDuration, 0, DebugLineThickness);

	// Draw arrow at target
	FVector Direction = (To - From).GetSafeNormal();
	DrawDebugDirectionalArrow(World, To - Direction * 50.0f, To, 30.0f, FColor::Magenta, false, DebugLineDuration, 0, DebugLineThickness);

	// Draw target marker
	DrawDebugSphere(World, To, 40.0f, 12, FColor::Magenta, false, DebugLineDuration, 0, 1.0f);
}

void UDebugVisualizer::DrawPerceptionRadius(const FVector& Center, float Radius)
{
	if (!bEnableVisualization)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Draw perception sphere
	DrawDebugSphere(World, Center, Radius, 32, FColor::Yellow, false, DebugLineDuration, 0, DebugLineThickness);

	// Draw perception cone (forward facing)
	FVector Forward = FVector::ForwardVector;
	float ConeAngle = 90.0f; // 180 degree field of view
	int32 NumSegments = 16;

	for (int32 i = 0; i < NumSegments; ++i)
	{
		float Angle1 = FMath::DegreesToRadians(ConeAngle * (float)i / NumSegments - ConeAngle / 2.0f);
		float Angle2 = FMath::DegreesToRadians(ConeAngle * (float)(i + 1) / NumSegments - ConeAngle / 2.0f);

		FVector Dir1 = FVector(FMath::Cos(Angle1), FMath::Sin(Angle1), 0.0f);
		FVector Dir2 = FVector(FMath::Cos(Angle2), FMath::Sin(Angle2), 0.0f);

		FVector Point1 = Center + Dir1 * Radius;
		FVector Point2 = Center + Dir2 * Radius;

		DrawDebugLine(World, Point1, Point2, FColor::Yellow, false, DebugLineDuration, 0, 1.0f);
	}

	// Draw center marker
	DrawDebugSphere(World, Center, 20.0f, 8, FColor::Orange, false, DebugLineDuration, 0, 1.0f);
}

void UDebugVisualizer::DrawNavMesh()
{
	if (!bEnableVisualization)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Get navigation system
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys)
	{
		return;
	}

	// Get the nav mesh
	ARecastNavMesh* NavMesh = Cast<ARecastNavMesh>(NavSys->GetDefaultNavDataInstance());
	if (!NavMesh)
	{
		return;
	}

	// Draw nav mesh bounds
	FBox NavBounds = NavMesh->GetNavMeshBounds();
	DrawDebugBox(World, NavBounds.GetCenter(), NavBounds.GetExtent(), FColor::Cyan, false, DebugLineDuration, 0, 2.0f);

	// Note: Drawing the full nav mesh geometry requires access to internal Recast data
	// For a production implementation, you would iterate through nav mesh tiles and polygons
	// Here we just draw the bounds as a simplified visualization
	
	UE_LOG(LogTemp, Log, TEXT("NavMesh bounds: %s"), *NavBounds.ToString());
}

void UDebugVisualizer::DrawHitboxes(AActor* Actor)
{
	if (!bEnableVisualization || !Actor)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Get actor bounds
	FVector Origin;
	FVector BoxExtent;
	Actor->GetActorBounds(false, Origin, BoxExtent);

	// Draw bounding box
	DrawDebugBox(World, Origin, BoxExtent, FColor::Cyan, false, DebugLineDuration, 0, DebugLineThickness);

	// Draw collision components
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* Primitive : PrimitiveComponents)
	{
		if (!Primitive || !Primitive->IsCollisionEnabled())
		{
			continue;
		}

		// Get component bounds
		FBoxSphereBounds Bounds = Primitive->Bounds;

		// Draw sphere bounds
		DrawDebugSphere(World, Bounds.Origin, Bounds.SphereRadius, 16, FColor::Green, false, DebugLineDuration, 0, 1.0f);

		// Draw box bounds
		DrawDebugBox(World, Bounds.Origin, Bounds.BoxExtent, FColor::Yellow, false, DebugLineDuration, 0, 1.0f);

		// Draw component name
		FString ComponentName = Primitive->GetName();
		DrawDebugString(World, Bounds.Origin, ComponentName, nullptr, FColor::White, DebugLineDuration, true, 0.8f);
	}

	// Draw actor location
	DrawDebugSphere(World, Actor->GetActorLocation(), 15.0f, 8, FColor::Red, false, DebugLineDuration, 0, 2.0f);

	// Draw actor forward vector
	FVector Forward = Actor->GetActorForwardVector() * 100.0f;
	DrawDebugDirectionalArrow(World, Actor->GetActorLocation(), Actor->GetActorLocation() + Forward, 
		20.0f, FColor::Red, false, DebugLineDuration, 0, 2.0f);
}

void UDebugVisualizer::DrawPredictedPath(const TArray<FVector>& Path, FColor Color)
{
	if (!bEnableVisualization || Path.Num() < 2)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Draw path segments
	for (int32 i = 0; i < Path.Num() - 1; ++i)
	{
		DrawDebugLine(World, Path[i], Path[i + 1], Color, false, DebugLineDuration, 0, DebugLineThickness);
		
		// Draw point markers
		DrawDebugSphere(World, Path[i], 8.0f, 8, Color, false, DebugLineDuration, 0, 1.0f);
	}

	// Draw final point
	if (Path.Num() > 0)
	{
		DrawDebugSphere(World, Path.Last(), 12.0f, 8, Color, false, DebugLineDuration, 0, 1.5f);
	}

	// Draw start marker
	if (Path.Num() > 0)
	{
		DrawDebugSphere(World, Path[0], 15.0f, 8, FColor::White, false, DebugLineDuration, 0, 2.0f);
	}
}

void UDebugVisualizer::DrawReconciledPath(const TArray<FVector>& Path, FColor Color)
{
	if (!bEnableVisualization || Path.Num() < 2)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Draw path segments with dashed line effect
	for (int32 i = 0; i < Path.Num() - 1; ++i)
	{
		// Create dashed line by drawing segments
		FVector Start = Path[i];
		FVector End = Path[i + 1];
		FVector Direction = End - Start;
		float Distance = Direction.Size();
		Direction.Normalize();

		float DashLength = 20.0f;
		float GapLength = 10.0f;
		float CurrentDistance = 0.0f;
		bool bDrawing = true;

		while (CurrentDistance < Distance)
		{
			float SegmentLength = bDrawing ? DashLength : GapLength;
			float NextDistance = FMath::Min(CurrentDistance + SegmentLength, Distance);

			if (bDrawing)
			{
				FVector SegmentStart = Start + Direction * CurrentDistance;
				FVector SegmentEnd = Start + Direction * NextDistance;
				DrawDebugLine(World, SegmentStart, SegmentEnd, Color, false, DebugLineDuration, 0, DebugLineThickness);
			}

			CurrentDistance = NextDistance;
			bDrawing = !bDrawing;
		}

		// Draw point markers
		DrawDebugSphere(World, Path[i], 6.0f, 8, Color, false, DebugLineDuration, 0, 1.0f);
	}

	// Draw final point
	if (Path.Num() > 0)
	{
		DrawDebugSphere(World, Path.Last(), 10.0f, 8, Color, false, DebugLineDuration, 0, 1.5f);
	}
}
