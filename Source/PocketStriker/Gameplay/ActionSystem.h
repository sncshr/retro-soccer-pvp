// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlayerStateMachine.h"
#include "ActionSystem.generated.h"

USTRUCT(BlueprintType)
struct FActionPreconditions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StaminaRequired;
};

USTRUCT(BlueprintType)
struct FActionEffects
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StaminaCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration;
};

USTRUCT(BlueprintType)
struct FActionDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPlayerAction ActionType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StaminaCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EPlayerState> ValidStates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FActionPreconditions Preconditions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FActionEffects Effects;
};

class ACharacter;
class UPlayerMovementComponent;

USTRUCT()
struct FActiveAction
{
	GENERATED_BODY()

	UPROPERTY()
	EPlayerAction ActionType;

	UPROPERTY()
	float RemainingTime;

	UPROPERTY()
	AActor* Instigator;
};

/**
 * Action system for managing player actions
 * Handles action registration, validation, and execution
 */
UCLASS()
class POCKETSTRIKER_API UActionSystem : public UObject
{
	GENERATED_BODY()

public:
	UActionSystem();

	void RegisterAction(const FActionDefinition& Action);
	bool TryExecuteAction(EPlayerAction Action, AActor* Instigator);
	void UpdateActiveActions(float DeltaTime);

	// Query active actions
	bool IsActionActive(EPlayerAction Action) const;
	float GetActionRemainingTime(EPlayerAction Action) const;

	// Apply tuning data
	void ApplyTuningData(class UPlayerTuningData* TuningData);

	// Ball interaction helpers
	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool ExecuteKickAction(AActor* Instigator, const FVector& Direction, float Force);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool ExecuteTackleAction(AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool ExecutePassAction(AActor* Instigator, AActor* TargetActor);

protected:
	void InitializeDefaultActions();
	const FActionDefinition* FindActionDefinition(EPlayerAction Action) const;
	bool ValidatePreconditions(const FActionDefinition& ActionDef, ACharacter* Character, UPlayerMovementComponent* MovementComp) const;
	void ExecuteActionEffects(const FActionDefinition& ActionDef, ACharacter* Character, UPlayerMovementComponent* MovementComp);
	void OnActionCompleted(const FActiveAction& CompletedAction);

	UPROPERTY()
	TArray<FActionDefinition> RegisteredActions;

	UPROPERTY()
	TArray<FActiveAction> ActiveActions;
};
