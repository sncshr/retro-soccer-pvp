// Copyright Epic Games, Inc. All Rights Reserved.

#include "Ball.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

ABall::ABall()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create sphere collision component
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(15.0f);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	SphereComponent->SetSimulatePhysics(true);
	SphereComponent->SetEnableGravity(true);
	SphereComponent->SetNotifyRigidBodyCollision(true);
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ABall::OnBallOverlap);

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));

	// Create possession effect
	PossessionEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PossessionEffect"));
	PossessionEffect->SetupAttachment(RootComponent);
	PossessionEffect->bAutoActivate = false;

	// Default physics properties
	BallMass = 0.45f; // Standard football mass in kg
	BallFriction = 0.5f;
	BallRestitution = 0.6f;
	BallLinearDamping = 0.5f;
	BallAngularDamping = 0.3f;

	// Default possession properties
	PossessionRadius = 100.0f;
	PossessionMinVelocity = 50.0f;

	// Default kick properties
	MaxKickForce = 2000.0f;
	PassAimAssistRadius = 500.0f;

	// Visual feedback
	PossessionHighlightColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
	bShowPossessionEffect = true;

	// Replication
	bReplicates = true;
	SetReplicateMovement(true);
	NetUpdateFrequency = 60.0f;

	PossessingActor = nullptr;
}

void ABall::BeginPlay()
{
	Super::BeginPlay();
	
	// Configure physics properties
	if (SphereComponent)
	{
		SphereComponent->SetMassOverrideInKg(NAME_None, BallMass, true);
		SphereComponent->SetPhysMaterialOverride(nullptr); // Use default physics material
		SphereComponent->SetLinearDamping(BallLinearDamping);
		SphereComponent->SetAngularDamping(BallAngularDamping);
	}

	UpdateVisualFeedback();
}

void ABall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdatePossession(DeltaTime);
}

void ABall::Kick(const FVector& Direction, float Force)
{
	if (!SphereComponent)
	{
		return;
	}

	// Clamp force to max
	float ClampedForce = FMath::Min(Force, MaxKickForce);

	// Apply impulse
	FVector Impulse = Direction.GetSafeNormal() * ClampedForce;
	SphereComponent->AddImpulse(Impulse, NAME_None, true);

	// Release possession
	ReleasePossession();

	UE_LOG(LogTemp, Log, TEXT("Ball: Kicked with force %.1f in direction %s"), 
		ClampedForce, *Direction.ToString());
}

void ABall::Pass(AActor* TargetActor, float Force)
{
	if (!TargetActor || !SphereComponent)
	{
		return;
	}

	// Calculate direction to target
	FVector TargetLocation = TargetActor->GetActorLocation();
	FVector CurrentLocation = GetActorLocation();
	FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();

	// Apply aim assist - adjust direction slightly towards target
	float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);
	if (DistanceToTarget < PassAimAssistRadius)
	{
		// Stronger aim assist for closer targets
		float AimAssistStrength = 1.0f - (DistanceToTarget / PassAimAssistRadius);
		Direction = FMath::Lerp(Direction, (TargetLocation - CurrentLocation).GetSafeNormal(), AimAssistStrength);
	}

	// Kick in the calculated direction
	Kick(Direction, Force);

	UE_LOG(LogTemp, Log, TEXT("Ball: Passed to target at distance %.1f"), DistanceToTarget);
}

bool ABall::TryGainPossession(AActor* NewOwner)
{
	if (!NewOwner)
	{
		return false;
	}

	// Check if ball is moving too fast to be possessed
	FVector Velocity = GetBallVelocity();
	if (Velocity.Size() > PossessionMinVelocity && PossessingActor != nullptr)
	{
		return false;
	}

	// Check distance to ball
	float Distance = FVector::Dist(GetActorLocation(), NewOwner->GetActorLocation());
	if (Distance > PossessionRadius)
	{
		return false;
	}

	// Gain possession
	PossessingActor = NewOwner;
	UpdateVisualFeedback();

	UE_LOG(LogTemp, Log, TEXT("Ball: Possession gained by %s"), *NewOwner->GetName());
	return true;
}

void ABall::ReleasePossession()
{
	if (PossessingActor)
	{
		UE_LOG(LogTemp, Log, TEXT("Ball: Possession released by %s"), *PossessingActor->GetName());
		PossessingActor = nullptr;
		UpdateVisualFeedback();
	}
}

FVector ABall::GetBallVelocity() const
{
	if (SphereComponent)
	{
		return SphereComponent->GetPhysicsLinearVelocity();
	}
	return FVector::ZeroVector;
}

FVector ABall::PredictPositionAtTime(float Time) const
{
	if (!SphereComponent)
	{
		return GetActorLocation();
	}

	// Simple ballistic prediction
	FVector CurrentLocation = GetActorLocation();
	FVector CurrentVelocity = GetBallVelocity();
	FVector Gravity = FVector(0.0f, 0.0f, GetWorld()->GetGravityZ());

	// Position = Current + Velocity*Time + 0.5*Gravity*Time^2
	FVector PredictedPosition = CurrentLocation + (CurrentVelocity * Time) + (0.5f * Gravity * Time * Time);

	return PredictedPosition;
}

void ABall::OnBallOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	// Check if overlapping with a character
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (Character)
	{
		// Attempt to gain possession if ball is slow enough
		TryGainPossession(Character);
	}
}

void ABall::UpdatePossession(float DeltaTime)
{
	if (!PossessingActor)
	{
		return;
	}

	// Check if possessing actor is still valid and close enough
	float Distance = FVector::Dist(GetActorLocation(), PossessingActor->GetActorLocation());
	if (Distance > PossessionRadius * 1.5f)
	{
		// Lost possession due to distance
		ReleasePossession();
		return;
	}

	// Keep ball near possessing actor
	FVector TargetLocation = PossessingActor->GetActorLocation() + FVector(50.0f, 0.0f, -50.0f);
	FVector CurrentLocation = GetActorLocation();
	FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, 10.0f);
	
	if (SphereComponent)
	{
		SphereComponent->SetWorldLocation(NewLocation);
		// Dampen velocity while possessed
		SphereComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
	}
}

void ABall::UpdateVisualFeedback()
{
	if (!bShowPossessionEffect || !PossessionEffect)
	{
		return;
	}

	if (PossessingActor)
	{
		// Show possession effect
		PossessionEffect->Activate(true);
	}
	else
	{
		// Hide possession effect
		PossessionEffect->Deactivate();
	}

	// Update mesh material color (if dynamic material is set up)
	if (MeshComponent)
	{
		// This would require a dynamic material instance
		// For now, just log the state change
		UE_LOG(LogTemp, Verbose, TEXT("Ball: Visual feedback updated - Possessed: %s"), 
			PossessingActor ? TEXT("Yes") : TEXT("No"));
	}
}

void ABall::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABall, PossessingActor);
}
