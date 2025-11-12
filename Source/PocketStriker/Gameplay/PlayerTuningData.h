// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerTuningData.generated.h"

/**
 * Data asset for player tuning parameters
 * Allows designers to adjust gameplay values without code changes
 */
UCLASS(BlueprintType)
class POCKETSTRIKER_API UPlayerTuningData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ToolTip = "Maximum walking speed in cm/s"))
	float MaxWalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ToolTip = "Maximum sprinting speed in cm/s"))
	float MaxSprintSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ToolTip = "Acceleration rate in cm/s²"))
	float Acceleration = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ToolTip = "Deceleration rate in cm/s²"))
	float Deceleration = 4000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (ToolTip = "Maximum stamina value"))
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (ToolTip = "Stamina consumed per second while sprinting"))
	float SprintStaminaCost = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina", meta = (ToolTip = "Stamina regeneration rate per second"))
	float StaminaRegenRate = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions", meta = (ToolTip = "Maximum range for tackle action in cm"))
	float TackleRange = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions", meta = (ToolTip = "Force applied when kicking the ball"))
	float KickForce = 2000.0f;
};
