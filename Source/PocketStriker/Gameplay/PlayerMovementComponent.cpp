// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerMovementComponent.h"
#include "GameplayTypes.h"
#include "PlayerTuningData.h"
#include "GameFramework/Character.h"

UPlayerMovementComponent::UPlayerMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// Initialize stamina to max
	CurrentStamina = 100.0f;
	
	// Set default movement parameters
	MaxWalkSpeed = 600.0f;
	MaxAcceleration = 2000.0f;
	BrakingDecelerationWalking = 4000.0f;
}

void UPlayerMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize stamina to max on begin play
	CurrentStamina = 100.0f;
}

void UPlayerMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Regenerate stamina when not sprinting
	if (!bIsSprinting)
	{
		RegenerateStamina(DeltaTime);
	}
}

void UPlayerMovementComponent::SimulateMovement(const FInputCommand& Input, float DeltaTime)
{
	// Deterministic movement simulation for network prediction
	// This method processes input and updates velocity/position
	
	if (!UpdatedComponent)
	{
		return;
	}

	// Get movement input direction
	FVector InputVector = FVector(Input.MovementInput.Y, Input.MovementInput.X, 0.0f);
	
	if (InputVector.SizeSquared() > 1.0f)
	{
		InputVector.Normalize();
	}

	// Check if sprinting
	bIsSprinting = (Input.ActionFlags & FInputCommand::FLAG_SPRINT) != 0;
	
	// Determine target speed based on sprint state
	float TargetSpeed = MaxWalkSpeed;
	if (bIsSprinting && CurrentStamina > 0.0f)
	{
		TargetSpeed = MaxSprintSpeed;
		ConsumeStamina(SprintStaminaCostPerSecond * DeltaTime);
	}
	else
	{
		bIsSprinting = false; // Can't sprint without stamina
	}

	// Calculate desired velocity
	FVector DesiredVelocity = InputVector * TargetSpeed;
	
	// Apply acceleration/deceleration
	FVector CurrentVelocity = Velocity;
	FVector VelocityDelta = DesiredVelocity - CurrentVelocity;
	
	float AccelRate = (InputVector.SizeSquared() > 0.0f) ? MaxAcceleration : BrakingDecelerationWalking;
	float MaxVelocityChange = AccelRate * DeltaTime;
	
	if (VelocityDelta.SizeSquared() > MaxVelocityChange * MaxVelocityChange)
	{
		VelocityDelta = VelocityDelta.GetSafeNormal() * MaxVelocityChange;
	}
	
	Velocity = CurrentVelocity + VelocityDelta;
	
	// Clamp velocity to max speed
	if (Velocity.SizeSquared() > TargetSpeed * TargetSpeed)
	{
		Velocity = Velocity.GetSafeNormal() * TargetSpeed;
	}

	// Apply velocity to position
	FVector Delta = Velocity * DeltaTime;
	if (!Delta.IsNearlyZero())
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);
		
		if (Hit.IsValidBlockingHit())
		{
			SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit);
		}
	}
}

void UPlayerMovementComponent::ConsumeStamina(float Amount)
{
	CurrentStamina = FMath::Max(0.0f, CurrentStamina - Amount);
	
	// If stamina depleted, stop sprinting
	if (CurrentStamina <= 0.0f)
	{
		bIsSprinting = false;
	}
}

void UPlayerMovementComponent::RegenerateStamina(float DeltaTime)
{
	CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (StaminaRegenRate * DeltaTime));
}

void UPlayerMovementComponent::ApplyRootMotionToVelocity(float DeltaTime)
{
	// Root motion handling for animation-driven movement
	// This will be integrated with the motion matching system
	
	if (HasAnimRootMotion() && DeltaTime > 0.0f)
	{
		// Get root motion from animation
		FTransform RootMotionTransform = ConsumeRootMotion();
		
		// Extract velocity from root motion
		FVector RootMotionVelocity = RootMotionTransform.GetTranslation() / DeltaTime;
		
		// Blend with current velocity
		// For now, we'll use root motion directly
		Velocity = RootMotionVelocity;
	}
}

void UPlayerMovementComponent::ApplyTuningData(const UPlayerTuningData* TuningData)
{
	if (!TuningData)
	{
		return;
	}

	// Apply tuning parameters
	MaxWalkSpeed = TuningData->MaxWalkSpeed;
	MaxSprintSpeed = TuningData->MaxSprintSpeed;
	MaxAcceleration = TuningData->Acceleration;
	BrakingDecelerationWalking = TuningData->Deceleration;
	
	MaxStamina = TuningData->MaxStamina;
	SprintStaminaCostPerSecond = TuningData->SprintStaminaCost;
	StaminaRegenRate = TuningData->StaminaRegenRate;
	
	// Initialize current stamina if not already set
	if (CurrentStamina > MaxStamina)
	{
		CurrentStamina = MaxStamina;
	}
}
