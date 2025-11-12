// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "DesignerPanelCommands.generated.h"

/**
 * Console commands for Designer Panel
 * Provides easy access to parameter editing functionality
 */
UCLASS()
class POCKETSTRIKER_API UDesignerPanelCommands : public UCheatManager
{
	GENERATED_BODY()

public:
	/**
	 * Toggle the designer panel UI
	 * Usage: ToggleDesignerPanel
	 */
	UFUNCTION(Exec, Category = "Designer Panel")
	void ToggleDesignerPanel();

	/**
	 * Load a parameter preset by name
	 * Usage: LoadDesignerPreset PresetName
	 */
	UFUNCTION(Exec, Category = "Designer Panel")
	void LoadDesignerPreset(const FString& PresetName);

	/**
	 * Save current parameters as a preset
	 * Usage: SaveDesignerPreset PresetName
	 */
	UFUNCTION(Exec, Category = "Designer Panel")
	void SaveDesignerPreset(const FString& PresetName);

	/**
	 * List all available presets
	 * Usage: ListDesignerPresets
	 */
	UFUNCTION(Exec, Category = "Designer Panel")
	void ListDesignerPresets();

	/**
	 * Reset all parameters to defaults
	 * Usage: ResetDesignerParameters
	 */
	UFUNCTION(Exec, Category = "Designer Panel")
	void ResetDesignerParameters();

	/**
	 * Set a specific parameter value
	 * Usage: SetDesignerParameter ParameterName Value
	 */
	UFUNCTION(Exec, Category = "Designer Panel")
	void SetDesignerParameter(const FString& ParameterName, float Value);

	/**
	 * Get a specific parameter value
	 * Usage: GetDesignerParameter ParameterName
	 */
	UFUNCTION(Exec, Category = "Designer Panel")
	void GetDesignerParameter(const FString& ParameterName);

private:
	class UDesignerPanel* GetDesignerPanel();
};
