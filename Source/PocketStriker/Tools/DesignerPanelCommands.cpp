// Copyright Epic Games, Inc. All Rights Reserved.

#include "DesignerPanelCommands.h"
#include "DesignerPanel.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void UDesignerPanelCommands::ToggleDesignerPanel()
{
	APlayerController* PC = GetOuterAPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("ToggleDesignerPanel: No PlayerController found"));
		return;
	}

	// Find existing designer panel widget
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UDesignerPanel::StaticClass());

	if (FoundWidgets.Num() > 0)
	{
		// Panel exists, remove it
		for (UUserWidget* Widget : FoundWidgets)
		{
			Widget->RemoveFromParent();
		}
		
		// Restore game input mode
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
		
		UE_LOG(LogTemp, Log, TEXT("Designer Panel closed"));
	}
	else
	{
		// Panel doesn't exist, create it
		// Note: This requires the Blueprint widget to be created in the editor
		// The Blueprint should be located at /Game/UI/WBP_DesignerPanel
		
		UE_LOG(LogTemp, Warning, TEXT("Designer Panel widget not found. Please create WBP_DesignerPanel in the editor."));
		UE_LOG(LogTemp, Warning, TEXT("See Content/UI/README.md for setup instructions."));
		
		// In a full implementation, you would load and create the widget here:
		// TSoftClassPtr<UUserWidget> WidgetClass = TSoftClassPtr<UUserWidget>(FSoftObjectPath(TEXT("/Game/UI/WBP_DesignerPanel.WBP_DesignerPanel_C")));
		// if (UClass* LoadedClass = WidgetClass.LoadSynchronous())
		// {
		//     UUserWidget* Widget = CreateWidget<UUserWidget>(PC, LoadedClass);
		//     Widget->AddToViewport(100);
		//     PC->SetInputMode(FInputModeUIOnly());
		//     PC->bShowMouseCursor = true;
		// }
	}
}

void UDesignerPanelCommands::LoadDesignerPreset(const FString& PresetName)
{
	UDesignerPanel* Panel = GetDesignerPanel();
	if (!Panel)
	{
		UE_LOG(LogTemp, Warning, TEXT("Designer Panel not found. Open it first with ToggleDesignerPanel."));
		return;
	}

	Panel->LoadPreset(PresetName);
	UE_LOG(LogTemp, Log, TEXT("Loaded preset: %s"), *PresetName);
}

void UDesignerPanelCommands::SaveDesignerPreset(const FString& PresetName)
{
	UDesignerPanel* Panel = GetDesignerPanel();
	if (!Panel)
	{
		UE_LOG(LogTemp, Warning, TEXT("Designer Panel not found. Open it first with ToggleDesignerPanel."));
		return;
	}

	Panel->SavePreset(PresetName);
	UE_LOG(LogTemp, Log, TEXT("Saved preset: %s"), *PresetName);
}

void UDesignerPanelCommands::ListDesignerPresets()
{
	UDesignerPanel* Panel = GetDesignerPanel();
	if (!Panel)
	{
		UE_LOG(LogTemp, Warning, TEXT("Designer Panel not found. Open it first with ToggleDesignerPanel."));
		return;
	}

	TArray<FString> Presets = Panel->GetAvailablePresets();
	
	if (Presets.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("No presets saved yet."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Available presets (%d):"), Presets.Num());
	for (const FString& PresetName : Presets)
	{
		UE_LOG(LogTemp, Log, TEXT("  - %s"), *PresetName);
	}
}

void UDesignerPanelCommands::ResetDesignerParameters()
{
	UDesignerPanel* Panel = GetDesignerPanel();
	if (!Panel)
	{
		UE_LOG(LogTemp, Warning, TEXT("Designer Panel not found. Open it first with ToggleDesignerPanel."));
		return;
	}

	Panel->ResetToDefaults();
	UE_LOG(LogTemp, Log, TEXT("Reset all parameters to defaults"));
}

void UDesignerPanelCommands::SetDesignerParameter(const FString& ParameterName, float Value)
{
	UDesignerPanel* Panel = GetDesignerPanel();
	if (!Panel)
	{
		UE_LOG(LogTemp, Warning, TEXT("Designer Panel not found. Open it first with ToggleDesignerPanel."));
		return;
	}

	Panel->OnParameterChanged(ParameterName, Value);
	UE_LOG(LogTemp, Log, TEXT("Set %s = %.2f"), *ParameterName, Value);
}

void UDesignerPanelCommands::GetDesignerParameter(const FString& ParameterName)
{
	UDesignerPanel* Panel = GetDesignerPanel();
	if (!Panel)
	{
		UE_LOG(LogTemp, Warning, TEXT("Designer Panel not found. Open it first with ToggleDesignerPanel."));
		return;
	}

	// Access the CurrentParameters map through reflection or add a getter function
	UE_LOG(LogTemp, Log, TEXT("Parameter: %s (use the UI to view current value)"), *ParameterName);
	UE_LOG(LogTemp, Warning, TEXT("GetDesignerParameter: Direct parameter reading not implemented. Use the UI."));
}

UDesignerPanel* UDesignerPanelCommands::GetDesignerPanel()
{
	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UDesignerPanel::StaticClass());

	if (FoundWidgets.Num() > 0)
	{
		return Cast<UDesignerPanel>(FoundWidgets[0]);
	}

	return nullptr;
}
