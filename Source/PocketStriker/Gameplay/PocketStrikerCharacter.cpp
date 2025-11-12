// Copyright Epic Games, Inc. All Rights Reserved.

#include "PocketStrikerCharacter.h"
#include "PlayerMovementComponent.h"
#include "PlayerStateMachine.h"
#include "PlayerTuningData.h"
#include "../Network/NetworkPrediction.h"
#include "../Network/NetworkReconciler.h"
#include "../Network/NetworkParamsData.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

APocketStrikerCharacter::APocketStrikerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UPlayerMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	// Configure capsule
	GetCapsuleComponent()->InitCapsuleSize(40.0f, 90.0f);

	// Configure mesh
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	// Create state machine
	StateMachine = CreateDefaultSubobject<UPlayerStateMachine>(TEXT("StateMachine"));

	// Create network components
	NetworkPrediction = CreateDefaultSubobject<UNetworkPrediction>(TEXT("NetworkPrediction"));
	NetworkReconciler = CreateDefaultSubobject<UNetworkReconciler>(TEXT("NetworkReconciler"));

	// Configure replication
	bReplicates = true;
	SetReplicateMovement(true);
	NetUpdateFrequency = 60.0f;
	MinNetUpdateFrequency = 30.0f;
}

void APocketStrikerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Apply tuning data
	ApplyTuningData();
}

void APocketStrikerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update state machine
	if (StateMachine)
	{
		StateMachine->UpdateState(DeltaTime);
	}
}

void APocketStrikerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Input binding is handled by PocketStrikerPlayerController
}

void APocketStrikerCharacter::ApplyTuningData()
{
	if (!PlayerTuning)
	{
		UE_LOG(LogTemp, Warning, TEXT("PocketStrikerCharacter: PlayerTuning data asset not assigned"));
		return;
	}

	// Apply movement parameters
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (MovementComp)
	{
		MovementComp->MaxWalkSpeed = PlayerTuning->MaxWalkSpeed;
		MovementComp->MaxAcceleration = PlayerTuning->Acceleration;
		MovementComp->BrakingDecelerationWalking = PlayerTuning->Deceleration;
		
		UE_LOG(LogTemp, Log, TEXT("PocketStrikerCharacter: Applied tuning data - MaxSpeed: %.1f, Accel: %.1f"), 
			PlayerTuning->MaxWalkSpeed, PlayerTuning->Acceleration);
	}

	// Apply network parameters
	if (NetworkParams && NetworkReconciler)
	{
		// Network parameters would be applied to network components here
		UE_LOG(LogTemp, Log, TEXT("PocketStrikerCharacter: Applied network parameters"));
	}
}
