// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DesignerPanel.generated.h"

class UPlayerTuningData;
class UAIParametersData;
class UNetworkParamsData;

/**
 * Parameter preset structure
 */
USTRUCT(BlueprintType)
struct FParameterPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PresetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, float> Parameters;

	FParameterPreset()
		: PresetName(TEXT("Default"))
	{
	}
};

/**
 * Designer parameter editor panel
 * Provides runtime editing of tunable gameplay parameters
 */
UCLASS()
class POCKETSTRIKER_API UDesignerPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	UDesignerPanel(const FObjectInitializer& ObjectInitializer);

	// Parameter editing
	UFUNCTION(BlueprintCallable, Category = "Designer Panel")
	void ExposeParameter(const FString& Category, const FString& Name, float MinValue, float MaxValue);

	UFUNCTION(BlueprintCallable, Category = "Designer Panel")
	void OnParameterChanged(const FString& Name, float NewValue);

	// Preset management
	UFUNCTION(BlueprintCallable, Category = "Designer Panel")
	void SavePreset(const FString& PresetName);

	UFUNCTION(BlueprintCallable, Category = "Designer Panel")
	void LoadPreset(const FString& PresetName);

	UFUNCTION(BlueprintCallable, Category = "Designer Panel")
	TArray<FString> GetAvailablePresets() const;

	// Hot reload
	UFUNCTION(BlueprintCallable, Category = "Designer Panel")
	void ApplyChanges();

	UFUNCTION(BlueprintCallable, Category = "Designer Panel")
	void ResetToDefaults();

	// Initialize with data assets
	UFUNCTION(BlueprintCallable, Category = "Designer Panel")
	void InitializeWithDataAssets(UPlayerTuningData* PlayerData, UAIParametersData* AIData, UNetworkParamsData* NetworkData);

protected:
	virtual void NativeConstruct() override;

	// Exposed parameters
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parameters")
	TMap<FString, float> CurrentParameters;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parameters")
	TMap<FString, float> DefaultParameters;

	// Parameter metadata
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parameters")
	TMap<FString, FString> ParameterCategories;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parameters")
	TMap<FString, float> ParameterMinValues;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parameters")
	TMap<FString, float> ParameterMaxValues;

	// Presets
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Presets")
	TArray<FParameterPreset> SavedPresets;

	// Data asset references
	UPROPERTY()
	UPlayerTuningData* PlayerTuningData;

	UPROPERTY()
	UAIParametersData* AIParametersData;

	UPROPERTY()
	UNetworkParamsData* NetworkParamsData;

private:
	// Helper functions
	void RegisterPlayerParameters();
	void RegisterAIParameters();
	void RegisterNetworkParameters();
	void ApplyParameterToDataAsset(const FString& Name, float Value);
	float GetParameterFromDataAsset(const FString& Name) const;
};
