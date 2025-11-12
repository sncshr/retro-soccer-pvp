// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NetworkTypes.h"
#include "NetworkInterpolation.generated.h"

/**
 * Remote entity interpolation component
 * Smooths remote player rendering with state buffering
 * Used for rendering other players in multiplayer
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POCKETSTRIKER_API UNetworkInterpolation : public UActorComponent
{
	GENERATED_BODY()

public:
	UNetworkInterpolation();

	// State buffering
	void AddServerState(const FStateUpdatePacket& State);
	void UpdateInterpolation(float DeltaTime);
	
	// Interpolation
	FVector GetInterpolatedPosition(float RenderTime) const;
	FRotator GetInterpolatedRotation(float RenderTime) const;
	FVector GetInterpolatedVelocity(float RenderTime) const;
	
	// Extrapolation for packet loss
	FVector ExtrapolatePosition(float DeltaTime);

	// Apply network parameters
	void ApplyNetworkParams(float InInterpolationDelay, int32 InBufferSize);

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (ToolTip = "Interpolation delay in seconds (default 100ms)"))
	float InterpolationDelay = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (ToolTip = "Maximum size of state buffer"))
	int32 MaxBufferSize = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network", meta = (ToolTip = "Enable extrapolation when packets are lost"))
	bool bEnableExtrapolation = true;

	// Debug info
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
	int32 CurrentBufferSize = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
	float TimeSinceLastPacket = 0.0f;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// State buffer for interpolation
	UPROPERTY()
	TArray<FStateUpdatePacket> StateBuffer;

	// Timing
	float LastPacketTime;
	float CurrentRenderTime;

	// Helper functions
	void GetInterpolationStates(float RenderTime, FStateUpdatePacket& OutFrom, FStateUpdatePacket& OutTo, float& OutAlpha) const;
	void TrimOldStates();
};
