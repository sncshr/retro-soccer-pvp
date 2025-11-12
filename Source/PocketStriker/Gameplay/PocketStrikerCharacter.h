// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PocketStrikerCharacter.generated.h"

class UPlayerMovementComponent;
class UPlayerStateMachine;
class UNetworkPrediction;
class UNetworkReconciler;
class UPlayerTuningData;
class UNetworkParamsData;

/**
 * Base character class for Pocket Striker
 * Provides common functionality for both player and AI characters
 */
UCLASS()
class POCKETSTRIKER_API APocketStrikerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APocketStrikerCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Component getters
	UFUNCTION(BlueprintCallable, Category = "Character")
	UPlayerStateMachine* GetStateMachine() const { return StateMachine; }

	UFUNCTION(BlueprintCallable, Category = "Character")
	UNetworkPrediction* GetNetworkPrediction() const { return NetworkPrediction; }

	UFUNCTION(BlueprintCallable, Category = "Character")
	UNetworkReconciler* GetNetworkReconciler() const { return NetworkReconciler; }

	// Data asset getters
	UFUNCTION(BlueprintCallable, Category = "Character")
	UPlayerTuningData* GetPlayerTuning() const { return PlayerTuning; }

	UFUNCTION(BlueprintCallable, Category = "Character")
	UNetworkParamsData* GetNetworkParams() const { return NetworkParams; }

protected:
	/** Player state machine for action management */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPlayerStateMachine* StateMachine;

	/** Network prediction component (for player characters) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNetworkPrediction* NetworkPrediction;

	/** Network reconciliation component (for player characters) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNetworkReconciler* NetworkReconciler;

	/** Player tuning data asset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tuning")
	UPlayerTuningData* PlayerTuning;

	/** Network parameters data asset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tuning")
	UNetworkParamsData* NetworkParams;

	/** Apply tuning data to character components */
	UFUNCTION(BlueprintCallable, Category = "Character")
	virtual void ApplyTuningData();
};
