// Copyright Epic Games, Inc. All Rights Reserved.

#include "LevelSetupHelper.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

ALevelSetupHelper::ALevelSetupHelper()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// Default settings
	AISpawnTag = FName("AISpawn");
	bAutoSpawnAI = true;
}

void ALevelSetupHelper::BeginPlay()
{
	Super::BeginPlay();
	
	if (bAutoSpawnAI && AICharacterClass)
	{
		SpawnAICharacters();
	}
	else if (bAutoSpawnAI && !AICharacterClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("LevelSetupHelper: AutoSpawnAI is enabled but AICharacterClass is not set!"));
	}
}

void ALevelSetupHelper::SpawnAICharacters()
{
	if (!AICharacterClass)
	{
		UE_LOG(LogTemp, Error, TEXT("LevelSetupHelper: Cannot spawn AI - AICharacterClass is not set!"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("LevelSetupHelper: Cannot spawn AI - World is null!"));
		return;
	}

	// Get all AI spawn points
	TArray<ATargetPoint*> SpawnPoints = GetAISpawnPoints();
	
	if (SpawnPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("LevelSetupHelper: No Target Points found with tag '%s'"), *AISpawnTag.ToString());
		return;
	}

	// Spawn AI at each spawn point
	for (ATargetPoint* SpawnPoint : SpawnPoints)
	{
		if (!SpawnPoint)
		{
			continue;
		}

		FVector SpawnLocation = SpawnPoint->GetActorLocation();
		FRotator SpawnRotation = SpawnPoint->GetActorRotation();

		// Set up spawn parameters
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnParams.Owner = this;

		// Spawn the AI character
		APawn* SpawnedAI = World->SpawnActor<APawn>(AICharacterClass, SpawnLocation, SpawnRotation, SpawnParams);
		
		if (SpawnedAI)
		{
			SpawnedAICharacters.Add(SpawnedAI);
			UE_LOG(LogTemp, Log, TEXT("LevelSetupHelper: Spawned AI character at location %s"), *SpawnLocation.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("LevelSetupHelper: Failed to spawn AI character at location %s"), *SpawnLocation.ToString());
		}
	}

	UE_LOG(LogTemp, Log, TEXT("LevelSetupHelper: Spawned %d AI characters from %d spawn points"), 
		SpawnedAICharacters.Num(), SpawnPoints.Num());
}

TArray<ATargetPoint*> ALevelSetupHelper::GetAISpawnPoints() const
{
	TArray<ATargetPoint*> SpawnPoints;
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return SpawnPoints;
	}

	// Iterate through all Target Points in the level
	for (TActorIterator<ATargetPoint> It(World); It; ++It)
	{
		ATargetPoint* TargetPoint = *It;
		if (TargetPoint && TargetPoint->ActorHasTag(AISpawnTag))
		{
			SpawnPoints.Add(TargetPoint);
		}
	}

	return SpawnPoints;
}
