// Copyright Epic Games, Inc. All Rights Reserved.

#include "DesignerPanel.h"
#include "../Gameplay/PlayerTuningData.h"
#include "../AI/AIParametersData.h"
#include "../Network/NetworkParamsData.h"
#include "Kismet/GameplayStatics.h"

UDesignerPanel::UDesignerPanel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerTuningData = nullptr;
	AIParametersData = nullptr;
	NetworkParamsData = nullptr;
}

void UDesignerPanel::NativeConstruct()
{
	Super::NativeConstruct();

	// Widget construction happens here
	// In a full implementation, this would create UI elements dynamically
}

void UDesignerPanel::InitializeWithDataAssets(UPlayerTuningData* PlayerData, UAIParametersData* AIData, UNetworkParamsData* NetworkData)
{
	PlayerTuningData = PlayerData;
	AIParametersData = AIData;
	NetworkParamsData = NetworkData;

	// Register all parameters from data assets
	if (PlayerTuningData)
	{
		RegisterPlayerParameters();
	}

	if (AIParametersData)
	{
		RegisterAIParameters();
	}

	if (NetworkParamsData)
	{
		RegisterNetworkParameters();
	}

	// Store defaults
	DefaultParameters = CurrentParameters;
}

void UDesignerPanel::RegisterPlayerParameters()
{
	if (!PlayerTuningData)
	{
		return;
	}

	// Register movement parameters
	ExposeParameter(TEXT("Player.Movement"), TEXT("MaxWalkSpeed"), 300.0f, 1000.0f);
	CurrentParameters.Add(TEXT("MaxWalkSpeed"), PlayerTuningData->MaxWalkSpeed);

	ExposeParameter(TEXT("Player.Movement"), TEXT("MaxSprintSpeed"), 500.0f, 1500.0f);
	CurrentParameters.Add(TEXT("MaxSprintSpeed"), PlayerTuningData->MaxSprintSpeed);

	ExposeParameter(TEXT("Player.Movement"), TEXT("Acceleration"), 500.0f, 5000.0f);
	CurrentParameters.Add(TEXT("Acceleration"), PlayerTuningData->Acceleration);

	ExposeParameter(TEXT("Player.Movement"), TEXT("Deceleration"), 1000.0f, 8000.0f);
	CurrentParameters.Add(TEXT("Deceleration"), PlayerTuningData->Deceleration);

	// Register stamina parameters
	ExposeParameter(TEXT("Player.Stamina"), TEXT("MaxStamina"), 50.0f, 200.0f);
	CurrentParameters.Add(TEXT("MaxStamina"), PlayerTuningData->MaxStamina);

	ExposeParameter(TEXT("Player.Stamina"), TEXT("SprintStaminaCost"), 5.0f, 50.0f);
	CurrentParameters.Add(TEXT("SprintStaminaCost"), PlayerTuningData->SprintStaminaCost);

	ExposeParameter(TEXT("Player.Stamina"), TEXT("StaminaRegenRate"), 5.0f, 30.0f);
	CurrentParameters.Add(TEXT("StaminaRegenRate"), PlayerTuningData->StaminaRegenRate);

	// Register action parameters
	ExposeParameter(TEXT("Player.Actions"), TEXT("TackleRange"), 50.0f, 300.0f);
	CurrentParameters.Add(TEXT("TackleRange"), PlayerTuningData->TackleRange);

	ExposeParameter(TEXT("Player.Actions"), TEXT("KickForce"), 500.0f, 5000.0f);
	CurrentParameters.Add(TEXT("KickForce"), PlayerTuningData->KickForce);
}

void UDesignerPanel::RegisterAIParameters()
{
	if (!AIParametersData)
	{
		return;
	}

	// Register perception parameters
	ExposeParameter(TEXT("AI.Perception"), TEXT("PerceptionRadius"), 500.0f, 5000.0f);
	CurrentParameters.Add(TEXT("PerceptionRadius"), AIParametersData->PerceptionRadius);

	ExposeParameter(TEXT("AI.Perception"), TEXT("UpdateInterval"), 0.05f, 1.0f);
	CurrentParameters.Add(TEXT("UpdateInterval"), AIParametersData->UpdateInterval);

	// Register behavior parameters
	ExposeParameter(TEXT("AI.Behavior"), TEXT("Aggression"), 0.0f, 1.0f);
	CurrentParameters.Add(TEXT("Aggression"), AIParametersData->Aggression);

	ExposeParameter(TEXT("AI.Behavior"), TEXT("TacticalAwareness"), 0.0f, 1.0f);
	CurrentParameters.Add(TEXT("TacticalAwareness"), AIParametersData->TacticalAwareness);

	// Register movement parameters
	ExposeParameter(TEXT("AI.Movement"), TEXT("MaxSpeed"), 300.0f, 1000.0f);
	CurrentParameters.Add(TEXT("MaxSpeed"), AIParametersData->MaxSpeed);

	ExposeParameter(TEXT("AI.Movement"), TEXT("AvoidanceRadius"), 50.0f, 300.0f);
	CurrentParameters.Add(TEXT("AvoidanceRadius"), AIParametersData->AvoidanceRadius);
}

void UDesignerPanel::RegisterNetworkParameters()
{
	if (!NetworkParamsData)
	{
		return;
	}

	// Register prediction parameters
	ExposeParameter(TEXT("Network.Prediction"), TEXT("CorrectionThreshold"), 1.0f, 100.0f);
	CurrentParameters.Add(TEXT("CorrectionThreshold"), NetworkParamsData->CorrectionThreshold);

	ExposeParameter(TEXT("Network.Prediction"), TEXT("SmoothingSpeed"), 1.0f, 50.0f);
	CurrentParameters.Add(TEXT("SmoothingSpeed"), NetworkParamsData->SmoothingSpeed);

	// Register interpolation parameters
	ExposeParameter(TEXT("Network.Interpolation"), TEXT("InterpolationDelay"), 0.05f, 0.5f);
	CurrentParameters.Add(TEXT("InterpolationDelay"), NetworkParamsData->InterpolationDelay);

	ExposeParameter(TEXT("Network.Interpolation"), TEXT("StateBufferSize"), 10.0f, 100.0f);
	CurrentParameters.Add(TEXT("StateBufferSize"), static_cast<float>(NetworkParamsData->StateBufferSize));

	// Register debug parameters
	ExposeParameter(TEXT("Network.Debug"), TEXT("SimulatedLatency"), 0.0f, 500.0f);
	CurrentParameters.Add(TEXT("SimulatedLatency"), NetworkParamsData->SimulatedLatency);

	ExposeParameter(TEXT("Network.Debug"), TEXT("PacketLossPercentage"), 0.0f, 50.0f);
	CurrentParameters.Add(TEXT("PacketLossPercentage"), NetworkParamsData->PacketLossPercentage);
}

void UDesignerPanel::ExposeParameter(const FString& Category, const FString& Name, float MinValue, float MaxValue)
{
	ParameterCategories.Add(Name, Category);
	ParameterMinValues.Add(Name, MinValue);
	ParameterMaxValues.Add(Name, MaxValue);

	// In a full implementation, this would create a UI slider widget
	UE_LOG(LogTemp, Log, TEXT("Exposed parameter: %s.%s [%.2f - %.2f]"), *Category, *Name, MinValue, MaxValue);
}

void UDesignerPanel::OnParameterChanged(const FString& Name, float NewValue)
{
	// Clamp to valid range
	if (ParameterMinValues.Contains(Name) && ParameterMaxValues.Contains(Name))
	{
		NewValue = FMath::Clamp(NewValue, ParameterMinValues[Name], ParameterMaxValues[Name]);
	}

	// Update current value
	CurrentParameters.Add(Name, NewValue);

	// Apply to data asset immediately for hot-reload
	ApplyParameterToDataAsset(Name, NewValue);

	UE_LOG(LogTemp, Log, TEXT("Parameter changed: %s = %.2f"), *Name, NewValue);
}

void UDesignerPanel::ApplyParameterToDataAsset(const FString& Name, float Value)
{
	// Player parameters
	if (PlayerTuningData)
	{
		if (Name == TEXT("MaxWalkSpeed")) PlayerTuningData->MaxWalkSpeed = Value;
		else if (Name == TEXT("MaxSprintSpeed")) PlayerTuningData->MaxSprintSpeed = Value;
		else if (Name == TEXT("Acceleration")) PlayerTuningData->Acceleration = Value;
		else if (Name == TEXT("Deceleration")) PlayerTuningData->Deceleration = Value;
		else if (Name == TEXT("MaxStamina")) PlayerTuningData->MaxStamina = Value;
		else if (Name == TEXT("SprintStaminaCost")) PlayerTuningData->SprintStaminaCost = Value;
		else if (Name == TEXT("StaminaRegenRate")) PlayerTuningData->StaminaRegenRate = Value;
		else if (Name == TEXT("TackleRange")) PlayerTuningData->TackleRange = Value;
		else if (Name == TEXT("KickForce")) PlayerTuningData->KickForce = Value;
	}

	// AI parameters
	if (AIParametersData)
	{
		if (Name == TEXT("PerceptionRadius")) AIParametersData->PerceptionRadius = Value;
		else if (Name == TEXT("UpdateInterval")) AIParametersData->UpdateInterval = Value;
		else if (Name == TEXT("Aggression")) AIParametersData->Aggression = Value;
		else if (Name == TEXT("TacticalAwareness")) AIParametersData->TacticalAwareness = Value;
		else if (Name == TEXT("MaxSpeed")) AIParametersData->MaxSpeed = Value;
		else if (Name == TEXT("AvoidanceRadius")) AIParametersData->AvoidanceRadius = Value;
	}

	// Network parameters
	if (NetworkParamsData)
	{
		if (Name == TEXT("CorrectionThreshold")) NetworkParamsData->CorrectionThreshold = Value;
		else if (Name == TEXT("SmoothingSpeed")) NetworkParamsData->SmoothingSpeed = Value;
		else if (Name == TEXT("InterpolationDelay")) NetworkParamsData->InterpolationDelay = Value;
		else if (Name == TEXT("StateBufferSize")) NetworkParamsData->StateBufferSize = static_cast<int32>(Value);
		else if (Name == TEXT("SimulatedLatency")) NetworkParamsData->SimulatedLatency = Value;
		else if (Name == TEXT("PacketLossPercentage")) NetworkParamsData->PacketLossPercentage = Value;
	}
}

float UDesignerPanel::GetParameterFromDataAsset(const FString& Name) const
{
	// Player parameters
	if (PlayerTuningData)
	{
		if (Name == TEXT("MaxWalkSpeed")) return PlayerTuningData->MaxWalkSpeed;
		if (Name == TEXT("MaxSprintSpeed")) return PlayerTuningData->MaxSprintSpeed;
		if (Name == TEXT("Acceleration")) return PlayerTuningData->Acceleration;
		if (Name == TEXT("Deceleration")) return PlayerTuningData->Deceleration;
		if (Name == TEXT("MaxStamina")) return PlayerTuningData->MaxStamina;
		if (Name == TEXT("SprintStaminaCost")) return PlayerTuningData->SprintStaminaCost;
		if (Name == TEXT("StaminaRegenRate")) return PlayerTuningData->StaminaRegenRate;
		if (Name == TEXT("TackleRange")) return PlayerTuningData->TackleRange;
		if (Name == TEXT("KickForce")) return PlayerTuningData->KickForce;
	}

	// AI parameters
	if (AIParametersData)
	{
		if (Name == TEXT("PerceptionRadius")) return AIParametersData->PerceptionRadius;
		if (Name == TEXT("UpdateInterval")) return AIParametersData->UpdateInterval;
		if (Name == TEXT("Aggression")) return AIParametersData->Aggression;
		if (Name == TEXT("TacticalAwareness")) return AIParametersData->TacticalAwareness;
		if (Name == TEXT("MaxSpeed")) return AIParametersData->MaxSpeed;
		if (Name == TEXT("AvoidanceRadius")) return AIParametersData->AvoidanceRadius;
	}

	// Network parameters
	if (NetworkParamsData)
	{
		if (Name == TEXT("CorrectionThreshold")) return NetworkParamsData->CorrectionThreshold;
		if (Name == TEXT("SmoothingSpeed")) return NetworkParamsData->SmoothingSpeed;
		if (Name == TEXT("InterpolationDelay")) return NetworkParamsData->InterpolationDelay;
		if (Name == TEXT("StateBufferSize")) return static_cast<float>(NetworkParamsData->StateBufferSize);
		if (Name == TEXT("SimulatedLatency")) return NetworkParamsData->SimulatedLatency;
		if (Name == TEXT("PacketLossPercentage")) return NetworkParamsData->PacketLossPercentage;
	}

	return 0.0f;
}

void UDesignerPanel::SavePreset(const FString& PresetName)
{
	FParameterPreset Preset;
	Preset.PresetName = PresetName;
	Preset.Parameters = CurrentParameters;

	// Remove existing preset with same name
	SavedPresets.RemoveAll([PresetName](const FParameterPreset& P)
	{
		return P.PresetName == PresetName;
	});

	SavedPresets.Add(Preset);

	UE_LOG(LogTemp, Log, TEXT("Saved preset: %s with %d parameters"), *PresetName, CurrentParameters.Num());
}

void UDesignerPanel::LoadPreset(const FString& PresetName)
{
	// Find preset
	FParameterPreset* Preset = SavedPresets.FindByPredicate([PresetName](const FParameterPreset& P)
	{
		return P.PresetName == PresetName;
	});

	if (!Preset)
	{
		UE_LOG(LogTemp, Warning, TEXT("Preset not found: %s"), *PresetName);
		return;
	}

	// Apply all parameters from preset
	for (const TPair<FString, float>& Param : Preset->Parameters)
	{
		OnParameterChanged(Param.Key, Param.Value);
	}

	UE_LOG(LogTemp, Log, TEXT("Loaded preset: %s"), *PresetName);
}

TArray<FString> UDesignerPanel::GetAvailablePresets() const
{
	TArray<FString> PresetNames;
	
	for (const FParameterPreset& Preset : SavedPresets)
	{
		PresetNames.Add(Preset.PresetName);
	}

	return PresetNames;
}

void UDesignerPanel::ApplyChanges()
{
	// Apply all current parameters to data assets
	for (const TPair<FString, float>& Param : CurrentParameters)
	{
		ApplyParameterToDataAsset(Param.Key, Param.Value);
	}

	UE_LOG(LogTemp, Log, TEXT("Applied %d parameter changes"), CurrentParameters.Num());
}

void UDesignerPanel::ResetToDefaults()
{
	// Restore default values
	CurrentParameters = DefaultParameters;

	// Apply defaults to data assets
	ApplyChanges();

	UE_LOG(LogTemp, Log, TEXT("Reset to default parameters"));
}
