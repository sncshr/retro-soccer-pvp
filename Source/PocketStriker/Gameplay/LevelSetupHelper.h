// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TargetPoint.h"
#include "LevelSetupHelper.generated.h"

/**
 * Helper actor for setting up the demo level with AI spawning
 * Place this actor in the level to automatically spawn AI characters at Target Points
 */
UCLASS()
class POCKETSTRIKER_API ALevelSetupHelper : public AActor
{
	GENERATED_BODY()
	
public:	
	ALevelSetupHelper();

protected:
	virtual void BeginPlay() override;

public:
	/** Blueprint class to spawn for AI characters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<APawn> AICharacterClass;

	/** Tag to identify AI spawn points (Target Points with this tag will be used) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	FName AISpawnTag;

	/** Whether to automatically spawn AI on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	bool bAutoSpawnAI;

	/** Spawn AI characters at all Target Points with the specified tag */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void SpawnAICharacters();

	/** Get all Target Points with the AI spawn tag */
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	TArray<ATargetPoint*> GetAISpawnPoints() const;

private:
	/** Array of spawned AI characters for tracking */
	UPROPERTY()
	TArray<APawn*> SpawnedAICharacters;
};
