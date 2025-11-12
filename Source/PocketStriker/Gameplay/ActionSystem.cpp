// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionSystem.h"
#include "PlayerStateMachine.h"
#include "PlayerMovementComponent.h"
#include "PlayerTuningData.h"
#include "Ball.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UActionSystem::UActionSystem()
{
	// Initialize with default actions
	InitializeDefaultActions();
}

void UActionSystem::InitializeDefaultActions()
{
	// Register default actions with their definitions
	
	// Sprint action
	FActionDefinition SprintAction;
	SprintAction.ActionType = EPlayerAction::Sprint;
	SprintAction.Duration = 0.0f; // Continuous action
	SprintAction.StaminaCost = 0.0f; // Cost handled per-second by movement component
	SprintAction.ValidStates = { EPlayerState::Idle, EPlayerState::Move };
	SprintAction.Preconditions.MinRange = 0.0f;
	SprintAction.Preconditions.MaxRange = 0.0f;
	SprintAction.Preconditions.StaminaRequired = 1.0f; // Need at least some stamina
	SprintAction.Effects.StaminaCost = 0.0f;
	SprintAction.Effects.Duration = 0.0f;
	RegisterAction(SprintAction);

	// Tackle action
	FActionDefinition TackleAction;
	TackleAction.ActionType = EPlayerAction::Tackle;
	TackleAction.Duration = 0.5f;
	TackleAction.StaminaCost = 15.0f;
	TackleAction.ValidStates = { EPlayerState::Idle, EPlayerState::Move, EPlayerState::Sprint };
	TackleAction.Preconditions.MinRange = 0.0f;
	TackleAction.Preconditions.MaxRange = 150.0f; // Tackle range
	TackleAction.Preconditions.StaminaRequired = 15.0f;
	TackleAction.Effects.StaminaCost = 15.0f;
	TackleAction.Effects.Duration = 0.5f;
	RegisterAction(TackleAction);

	// Kick action
	FActionDefinition KickAction;
	KickAction.ActionType = EPlayerAction::Kick;
	KickAction.Duration = 0.4f;
	KickAction.StaminaCost = 10.0f;
	KickAction.ValidStates = { EPlayerState::Idle, EPlayerState::Move, EPlayerState::Sprint };
	KickAction.Preconditions.MinRange = 0.0f;
	KickAction.Preconditions.MaxRange = 100.0f; // Kick range
	KickAction.Preconditions.StaminaRequired = 10.0f;
	KickAction.Effects.StaminaCost = 10.0f;
	KickAction.Effects.Duration = 0.4f;
	RegisterAction(KickAction);

	// Pass action
	FActionDefinition PassAction;
	PassAction.ActionType = EPlayerAction::Pass;
	PassAction.Duration = 0.3f;
	PassAction.StaminaCost = 5.0f;
	PassAction.ValidStates = { EPlayerState::Idle, EPlayerState::Move, EPlayerState::Sprint };
	PassAction.Preconditions.MinRange = 0.0f;
	PassAction.Preconditions.MaxRange = 100.0f; // Pass range
	PassAction.Preconditions.StaminaRequired = 5.0f;
	PassAction.Effects.StaminaCost = 5.0f;
	PassAction.Effects.Duration = 0.3f;
	RegisterAction(PassAction);
}

void UActionSystem::RegisterAction(const FActionDefinition& Action)
{
	// Check if action already registered
	for (int32 i = 0; i < RegisteredActions.Num(); ++i)
	{
		if (RegisteredActions[i].ActionType == Action.ActionType)
		{
			// Update existing action
			RegisteredActions[i] = Action;
			return;
		}
	}

	// Add new action
	RegisteredActions.Add(Action);
}

bool UActionSystem::TryExecuteAction(EPlayerAction Action, AActor* Instigator)
{
	if (!Instigator)
	{
		return false;
	}

	// Find action definition
	const FActionDefinition* ActionDef = FindActionDefinition(Action);
	if (!ActionDef)
	{
		UE_LOG(LogTemp, Warning, TEXT("Action %d not registered"), static_cast<int32>(Action));
		return false;
	}

	// Get character components
	ACharacter* Character = Cast<ACharacter>(Instigator);
	if (!Character)
	{
		return false;
	}

	UPlayerMovementComponent* MovementComp = Cast<UPlayerMovementComponent>(Character->GetCharacterMovement());
	if (!MovementComp)
	{
		return false;
	}

	// Validate preconditions
	if (!ValidatePreconditions(*ActionDef, Character, MovementComp))
	{
		return false;
	}

	// Execute action
	ExecuteActionEffects(*ActionDef, Character, MovementComp);

	// Add to active actions if it has a duration
	if (ActionDef->Duration > 0.0f)
	{
		FActiveAction ActiveAction;
		ActiveAction.ActionType = Action;
		ActiveAction.RemainingTime = ActionDef->Duration;
		ActiveAction.Instigator = Instigator;
		ActiveActions.Add(ActiveAction);
	}

	return true;
}

void UActionSystem::UpdateActiveActions(float DeltaTime)
{
	// Update all active actions
	for (int32 i = ActiveActions.Num() - 1; i >= 0; --i)
	{
		FActiveAction& ActiveAction = ActiveActions[i];
		ActiveAction.RemainingTime -= DeltaTime;

		// Remove completed actions
		if (ActiveAction.RemainingTime <= 0.0f)
		{
			OnActionCompleted(ActiveAction);
			ActiveActions.RemoveAt(i);
		}
	}
}

const FActionDefinition* UActionSystem::FindActionDefinition(EPlayerAction Action) const
{
	for (const FActionDefinition& ActionDef : RegisteredActions)
	{
		if (ActionDef.ActionType == Action)
		{
			return &ActionDef;
		}
	}
	return nullptr;
}

bool UActionSystem::ValidatePreconditions(const FActionDefinition& ActionDef, ACharacter* Character, UPlayerMovementComponent* MovementComp) const
{
	// Check stamina requirement
	if (MovementComp->CurrentStamina < ActionDef.Preconditions.StaminaRequired)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Insufficient stamina for action"));
		return false;
	}

	// Range checks would require a target actor
	// For now, we'll skip range validation as it depends on game context
	// This will be implemented when ball/opponent targeting is added

	return true;
}

void UActionSystem::ExecuteActionEffects(const FActionDefinition& ActionDef, ACharacter* Character, UPlayerMovementComponent* MovementComp)
{
	// Apply stamina cost
	if (ActionDef.Effects.StaminaCost > 0.0f)
	{
		MovementComp->ConsumeStamina(ActionDef.Effects.StaminaCost);
	}

	// Action-specific effects
	switch (ActionDef.ActionType)
	{
		case EPlayerAction::Sprint:
			// Sprint is handled by movement component
			break;

		case EPlayerAction::Tackle:
			UE_LOG(LogTemp, Log, TEXT("Executing Tackle action"));
			// Tackle logic will be implemented when ball/opponent system is added
			break;

		case EPlayerAction::Kick:
			UE_LOG(LogTemp, Log, TEXT("Executing Kick action"));
			// Kick logic will be implemented when ball system is added
			break;

		case EPlayerAction::Pass:
			UE_LOG(LogTemp, Log, TEXT("Executing Pass action"));
			// Pass logic will be implemented when ball system is added
			break;

		default:
			break;
	}
}

void UActionSystem::OnActionCompleted(const FActiveAction& CompletedAction)
{
	UE_LOG(LogTemp, Verbose, TEXT("Action %d completed"), static_cast<int32>(CompletedAction.ActionType));
	
	// Notify instigator that action is complete
	// This could trigger state machine transitions or other gameplay events
}

bool UActionSystem::IsActionActive(EPlayerAction Action) const
{
	for (const FActiveAction& ActiveAction : ActiveActions)
	{
		if (ActiveAction.ActionType == Action)
		{
			return true;
		}
	}
	return false;
}

float UActionSystem::GetActionRemainingTime(EPlayerAction Action) const
{
	for (const FActiveAction& ActiveAction : ActiveActions)
	{
		if (ActiveAction.ActionType == Action)
		{
			return ActiveAction.RemainingTime;
		}
	}
	return 0.0f;
}

void UActionSystem::ApplyTuningData(UPlayerTuningData* TuningData)
{
	if (!TuningData)
	{
		return;
	}

	// Update tackle action with tuning data
	for (FActionDefinition& ActionDef : RegisteredActions)
	{
		if (ActionDef.ActionType == EPlayerAction::Tackle)
		{
			ActionDef.Preconditions.MaxRange = TuningData->TackleRange;
		}
		// KickForce will be used when ball physics is implemented
		// For now, we just store it in the tuning data
	}

	UE_LOG(LogTemp, Log, TEXT("Applied TuningData to ActionSystem"));
}

bool UActionSystem::ExecuteKickAction(AActor* Instigator, const FVector& Direction, float Force)
{
	if (!Instigator)
	{
		return false;
	}

	// Try to execute kick action first
	if (!TryExecuteAction(EPlayerAction::Kick, Instigator))
	{
		return false;
	}

	// Find nearby ball
	// This is a simplified implementation - in a full game, you'd have a ball manager
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(Instigator->GetWorld(), ABall::StaticClass(), FoundActors);

	ABall* NearestBall = nullptr;
	float NearestDistance = 200.0f; // Max kick distance

	for (AActor* Actor : FoundActors)
	{
		ABall* Ball = Cast<ABall>(Actor);
		if (Ball)
		{
			float Distance = FVector::Dist(Instigator->GetActorLocation(), Ball->GetActorLocation());
			if (Distance < NearestDistance)
			{
				NearestBall = Ball;
				NearestDistance = Distance;
			}
		}
	}

	if (NearestBall)
	{
		// Kick the ball
		NearestBall->Kick(Direction, Force);
		UE_LOG(LogTemp, Log, TEXT("ActionSystem: Kicked ball with force %.1f"), Force);
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("ActionSystem: No ball found within kick range"));
	return false;
}

bool UActionSystem::ExecuteTackleAction(AActor* Instigator)
{
	if (!Instigator)
	{
		return false;
	}

	// Try to execute tackle action first
	if (!TryExecuteAction(EPlayerAction::Tackle, Instigator))
	{
		return false;
	}

	// Find nearby ball
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(Instigator->GetWorld(), ABall::StaticClass(), FoundActors);

	ABall* NearestBall = nullptr;
	float NearestDistance = 150.0f; // Tackle range

	for (AActor* Actor : FoundActors)
	{
		ABall* Ball = Cast<ABall>(Actor);
		if (Ball)
		{
			float Distance = FVector::Dist(Instigator->GetActorLocation(), Ball->GetActorLocation());
			if (Distance < NearestDistance)
			{
				NearestBall = Ball;
				NearestDistance = Distance;
			}
		}
	}

	if (NearestBall)
	{
		// Try to gain possession
		if (NearestBall->TryGainPossession(Instigator))
		{
			UE_LOG(LogTemp, Log, TEXT("ActionSystem: Tackle successful - gained possession"));
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("ActionSystem: Tackle failed - could not gain possession"));
			return false;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("ActionSystem: No ball found within tackle range"));
	return false;
}

bool UActionSystem::ExecutePassAction(AActor* Instigator, AActor* TargetActor)
{
	if (!Instigator || !TargetActor)
	{
		return false;
	}

	// Try to execute pass action first
	if (!TryExecuteAction(EPlayerAction::Pass, Instigator))
	{
		return false;
	}

	// Find nearby ball
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(Instigator->GetWorld(), ABall::StaticClass(), FoundActors);

	ABall* NearestBall = nullptr;
	float NearestDistance = 100.0f; // Pass range

	for (AActor* Actor : FoundActors)
	{
		ABall* Ball = Cast<ABall>(Actor);
		if (Ball)
		{
			float Distance = FVector::Dist(Instigator->GetActorLocation(), Ball->GetActorLocation());
			if (Distance < NearestDistance)
			{
				NearestBall = Ball;
				NearestDistance = Distance;
			}
		}
	}

	if (NearestBall)
	{
		// Pass to target with moderate force
		float PassForce = 1000.0f; // Default pass force
		NearestBall->Pass(TargetActor, PassForce);
		UE_LOG(LogTemp, Log, TEXT("ActionSystem: Passed ball to target"));
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("ActionSystem: No ball found within pass range"));
	return false;
}
