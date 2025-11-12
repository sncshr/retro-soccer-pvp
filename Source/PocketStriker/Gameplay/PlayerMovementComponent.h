// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerMovementComponent.generated.h"

struct FInputCommand;
class UPlayerTuningData;

/**
 * Custom movement component for player character
 * Implements deterministic movement simulation and stamina system
 */
UCLASS()
class POCKETSTRIKER_API UPlayerMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UPlayerMovementComponent();

	// Movement simulation (deterministic for prediction)
	void SimulateMovement(const FInputCommand& Input, float DeltaTime);
	
	// Stamina system
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
	float CurrentStamina;
	
	void ConsumeStamina(float Amount);
	void RegenerateStamina(float DeltaTime);
	
	// Root motion handling
	void ApplyRootMotionToVelocity(float DeltaTime);

	// Apply tuning data
	void ApplyTuningData(const UPlayerTuningData* TuningData);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Movement parameters
	UPROPERTY()
	float MaxSprintSpeed = 900.0f;

	// Stamina parameters
	UPROPERTY()
	float MaxStamina = 100.0f;

	UPROPERTY()
	float SprintStaminaCostPerSecond = 20.0f;

	UPROPERTY()
	float StaminaRegenRate = 15.0f;

	// State
	bool bIsSprinting = false;
};
