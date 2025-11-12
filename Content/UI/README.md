# Designer Panel UI Setup Guide

This guide explains how to create the Blueprint widget for the Designer Parameter Editor UI.

## Overview

The Designer Panel provides runtime editing of gameplay parameters through a user-friendly interface. The C++ backend (`UDesignerPanel`) is already implemented in `Source/PocketStriker/Tools/DesignerPanel.h/cpp`.

## Creating the Blueprint Widget

### Step 1: Create the Widget Blueprint

1. Open the Unreal Editor
2. In the Content Browser, navigate to `Content/UI/`
3. Right-click and select **User Interface > Widget Blueprint**
4. Name it `WBP_DesignerPanel`
5. Open the widget blueprint

### Step 2: Set Parent Class

1. In the widget blueprint editor, click **File > Reparent Blueprint**
2. Search for and select `DesignerPanel` (the C++ class)
3. Click **Select**

### Step 3: Design the UI Layout

Create the following UI structure in the Designer tab:

```
Canvas Panel (Root)
├── Vertical Box (Main Container)
│   ├── Text Block (Title: "Designer Parameter Editor")
│   ├── Horizontal Box (Preset Controls)
│   │   ├── Combo Box (String) - Name: "PresetDropdown"
│   │   ├── Button - Name: "SavePresetButton" (Text: "Save Preset")
│   │   ├── Button - Name: "LoadPresetButton" (Text: "Load Preset")
│   │   └── Button - Name: "ResetButton" (Text: "Reset to Defaults")
│   ├── Scroll Box (Parameter Container)
│   │   ├── Vertical Box - Name: "PlayerParametersBox"
│   │   ├── Vertical Box - Name: "AIParametersBox"
│   │   └── Vertical Box - Name: "NetworkParametersBox"
│   └── Button - Name: "ApplyButton" (Text: "Apply Changes")
```

### Step 4: Create Parameter Slider Widget

Create a reusable parameter slider widget:

1. Create a new Widget Blueprint named `WBP_ParameterSlider`
2. Add the following structure:

```
Horizontal Box (Root)
├── Text Block - Name: "ParameterNameText" (Width: 200px)
├── Slider - Name: "ParameterSlider" (Width: 300px)
├── Editable Text Box - Name: "ParameterValueText" (Width: 80px)
└── Text Block - Name: "ModifiedIndicator" (Text: "*", Visibility: Hidden)
```

3. Add variables:
   - `ParameterName` (String)
   - `MinValue` (Float)
   - `MaxValue` (Float)
   - `CurrentValue` (Float)
   - `IsModified` (Boolean)

4. Create functions:
   - `Initialize(Name, Min, Max, Current)` - Sets up the slider
   - `OnSliderChanged(Value)` - Updates text and fires event
   - `OnTextChanged(Text)` - Updates slider from text input
   - `SetModified(IsModified)` - Shows/hides the modified indicator

5. Add Event Dispatcher: `OnValueChanged(ParameterName, NewValue)`

### Step 5: Implement WBP_DesignerPanel Logic

In the Event Graph of `WBP_DesignerPanel`:

#### Event Construct
```
Event Construct
├── Create Widget (WBP_ParameterSlider) for each parameter
├── Add to appropriate category box (Player/AI/Network)
├── Bind OnValueChanged event to call OnParameterChanged
└── Populate PresetDropdown with GetAvailablePresets
```

#### Button Events
```
SavePresetButton OnClicked
└── Get text from PresetDropdown
└── Call SavePreset(PresetName)

LoadPresetButton OnClicked
└── Get selected preset from PresetDropdown
└── Call LoadPreset(PresetName)
└── Update all slider widgets with new values

ResetButton OnClicked
└── Call ResetToDefaults
└── Update all slider widgets with default values

ApplyButton OnClicked
└── Call ApplyChanges
└── Clear all modified indicators
```

### Step 6: Initialize with Data Assets

Create a function `InitializePanel` that:

1. Loads the three DataAssets:
   - `DA_PlayerTuning` (PlayerTuningData)
   - `DA_AIParameters` (AIParametersData)
   - `DA_NetworkParams` (NetworkParamsData)

2. Calls `InitializeWithDataAssets(PlayerData, AIData, NetworkData)`

3. Creates parameter sliders for all exposed parameters:

**Player Parameters:**
- MaxWalkSpeed (300-1000)
- MaxSprintSpeed (500-1500)
- Acceleration (500-5000)
- Deceleration (1000-8000)
- MaxStamina (50-200)
- SprintStaminaCost (5-50)
- StaminaRegenRate (5-30)
- TackleRange (50-300)
- KickForce (500-5000)

**AI Parameters:**
- PerceptionRadius (500-5000)
- UpdateInterval (0.05-1.0)
- Aggression (0-1)
- TacticalAwareness (0-1)
- MaxSpeed (300-1000)
- AvoidanceRadius (50-300)

**Network Parameters:**
- CorrectionThreshold (1-100)
- SmoothingSpeed (1-50)
- InterpolationDelay (0.05-0.5)
- StateBufferSize (10-100)
- SimulatedLatency (0-500)
- PacketLossPercentage (0-50)

### Step 7: Add Visual Feedback

Implement visual feedback for parameter changes:

1. When a parameter is changed, set `IsModified` to true on the slider widget
2. Show the modified indicator (asterisk or color change)
3. When ApplyChanges is called, clear all modified indicators
4. Add color coding:
   - Normal: White
   - Modified: Yellow
   - Out of range: Red

### Step 8: Add to HUD or Game UI

To display the panel in-game:

1. Create a toggle key binding (e.g., F2) in your PlayerController
2. In the toggle function:
   ```
   If DesignerPanelWidget is null:
       Create Widget (WBP_DesignerPanel)
       Call InitializePanel
       Add to Viewport
   Else:
       Remove from Parent
       Set to null
   ```

## Testing Hot-Reload Functionality

1. Launch the game in PIE (Play In Editor)
2. Press the toggle key to open the Designer Panel
3. Modify a parameter (e.g., MaxWalkSpeed)
4. Observe the change takes effect immediately without restarting
5. Test preset save/load functionality
6. Test reset to defaults

## Styling Recommendations

- Use a semi-transparent dark background for the panel
- Group parameters by category with collapsible sections
- Use tooltips to show parameter descriptions
- Add min/max value labels on sliders
- Use consistent spacing and alignment
- Make the panel draggable for better UX

## Console Commands

Add these console commands for testing:

```cpp
// In DesignerPanel.cpp or a debug console class
UFUNCTION(Exec)
void ToggleDesignerPanel();

UFUNCTION(Exec)
void LoadPreset(FString PresetName);

UFUNCTION(Exec)
void SavePreset(FString PresetName);
```

## Troubleshooting

**Panel doesn't appear:**
- Check that the widget is added to viewport
- Verify the parent class is set correctly
- Check Z-order (should be on top)

**Parameters don't update:**
- Verify OnParameterChanged is being called
- Check that DataAssets are properly loaded
- Ensure ApplyChanges is called after modifications

**Hot-reload doesn't work:**
- Verify DataAssets are not marked as const
- Check that the DataAsset references are valid
- Ensure the game is running in PIE mode

## Advanced Features (Optional)

- Add search/filter functionality for parameters
- Implement undo/redo for parameter changes
- Add parameter comparison view (current vs default)
- Export/import presets to JSON files
- Add parameter animation/tweening for smooth transitions
- Implement parameter groups with enable/disable toggles
