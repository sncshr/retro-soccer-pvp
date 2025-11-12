// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "PocketStrikerPlayerController.generated.h"

class UPlayerTuningData;
class UInputAction;
class UInputMappingContext;
class UActionSystem;
class UPerformanceProfiler;
struct FInputCommand;

/**
 * Player controller for Pocket Striker
 * Handles input processing, state management, and network prediction
 */
UCLASS()
class POCKETSTRIKER_API APocketStrikerPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APocketStrikerPlayerController();

	// Input processing
	void ProcessInput(float DeltaTime);
	void BufferInputCommand(const FInputCommand& Command);
	
	// Network prediction
	TArray<FInputCommand> GetUnacknowledgedInputs() const;
	void AcknowledgeInput(uint32 SequenceNumber);

	// Network RPCs for client-server communication
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSendInput(const FInputCommand& Input);

	UFUNCTION(Client, Reliable)
	void ClientReceiveStateUpdate(const FVector& Position, const FVector& Velocity, float Stamina, uint8 State, uint32 AckedSequence);
	
	// Exposed parameters
	UPROPERTY(EditDefaultsOnly, Category = "Tuning", meta = (ToolTip = "Player tuning data asset for gameplay parameters"))
	UPlayerTuningData* TuningData;

	// Action system
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
	UActionSystem* ActionSystem;

	// Performance profiler (optional, for tracking input latency)
	UPROPERTY()
	UPerformanceProfiler* PerformanceProfiler;

	// Apply tuning data to gameplay systems
	UFUNCTION(BlueprintCallable, Category = "Tuning")
	void ApplyTuningData();

	// Hot-reload support - called when DataAsset changes
	UFUNCTION(BlueprintCallable, Category = "Tuning")
	void OnTuningDataChanged();

	// Input Actions (to be set in Blueprint or data asset)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* TackleAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* KickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* PassAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* InputMappingContext;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	// Input callbacks
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Sprint();
	void StopSprint();
	void Tackle();
	void Kick();
	void Pass();

private:
	// Input buffering for network prediction
	TArray<FInputCommand> InputBuffer;
	uint32 CurrentInputSequence = 0;
	uint32 LastAcknowledgedSequence = 0;
	
	// Input state
	bool bIsSprintPressed = false;

#if WITH_EDITOR
	// Hot-reload support for DataAsset changes
	void OnDataAssetPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);
#endif
};
