# WBP_DesignerPanel Blueprint Implementation Guide

This document provides detailed step-by-step instructions for implementing the Designer Panel widget in Unreal Engine's Blueprint editor.

## Part 1: WBP_ParameterSlider (Reusable Component)

### Widget Hierarchy
```
Horizontal Box (Root)
├── Text Block (ParameterNameText)
│   └── Size: 200px width
│   └── Text: "Parameter Name"
│   └── Font Size: 12
├── Slider (ParameterSlider)
│   └── Size: 300px width
│   └── Min Value: 0.0
│   └── Max Value: 1.0
│   └── Step Size: 0.01
├── Editable Text Box (ParameterValueText)
│   └── Size: 80px width
│   └── Text: "0.00"
│   └── Font Size: 12
└── Text Block (ModifiedIndicator)
    └── Text: "*"
    └── Color: Yellow
    └── Visibility: Hidden
    └── Font Size: 16
```

### Variables
```
ParameterName (String) - Editable, Instance Editable
MinValue (Float) - Editable, Instance Editable
MaxValue (Float) - Editable, Instance Editable
CurrentValue (Float) - Editable, Instance Editable
DefaultValue (Float) - Editable, Instance Editable
IsModified (Boolean) - Default: false
```

### Event Dispatcher
```
OnValueChanged (ParameterName: String, NewValue: Float)
```

### Functions

#### Initialize
```
Inputs: Name (String), Min (Float), Max (Float), Current (Float)
Nodes:
1. Set ParameterName = Name
2. Set MinValue = Min
3. Set MaxValue = Max
4. Set CurrentValue = Current
5. Set DefaultValue = Current
6. Set ParameterNameText.Text = Name
7. Set ParameterSlider.MinValue = Min
8. Set ParameterSlider.MaxValue = Max
9. Set ParameterSlider.Value = Current
10. Format String: "{0:.2f}" with Current
11. Set ParameterValueText.Text = Formatted String
12. Set IsModified = false
13. Set ModifiedIndicator.Visibility = Hidden
```

#### UpdateValue
```
Inputs: NewValue (Float)
Nodes:
1. Clamp NewValue between MinValue and MaxValue
2. Set CurrentValue = Clamped Value
3. Set ParameterSlider.Value = Clamped Value
4. Format String: "{0:.2f}" with Clamped Value
5. Set ParameterValueText.Text = Formatted String
6. Compare: CurrentValue != DefaultValue
7. Set IsModified = Comparison Result
8. Branch on IsModified:
   - True: Set ModifiedIndicator.Visibility = Visible
   - False: Set ModifiedIndicator.Visibility = Hidden
9. Call Event Dispatcher: OnValueChanged(ParameterName, CurrentValue)
```

#### ResetToDefault
```
Nodes:
1. Call UpdateValue(DefaultValue)
```

### Events

#### ParameterSlider OnValueChanged
```
Nodes:
1. Get Slider Value
2. Call UpdateValue(Slider Value)
```

#### ParameterValueText OnTextCommitted
```
Nodes:
1. Get Text
2. Convert String to Float
3. Branch on Conversion Success:
   - True: Call UpdateValue(Float Value)
   - False: Restore previous value to text box
```

---

## Part 2: WBP_DesignerPanel (Main Panel)

### Widget Hierarchy
```
Canvas Panel (Root)
└── Border (Background)
    └── Padding: 10
    └── Background Color: (0, 0, 0, 0.8)
    └── Vertical Box (MainContainer)
        ├── Text Block (TitleText)
        │   └── Text: "Designer Parameter Editor"
        │   └── Font Size: 24
        │   └── Justification: Center
        ├── Spacer (Height: 10)
        ├── Horizontal Box (PresetControls)
        │   ├── Text Block (Label: "Preset:")
        │   ├── Combo Box String (PresetDropdown)
        │   │   └── Width: 200px
        │   ├── Button (SavePresetButton)
        │   │   └── Text: "Save"
        │   ├── Button (LoadPresetButton)
        │   │   └── Text: "Load"
        │   └── Button (ResetButton)
        │       └── Text: "Reset to Defaults"
        ├── Spacer (Height: 10)
        ├── Scroll Box (ParameterScrollBox)
        │   └── Vertical Box (ParameterContainer)
        │       ├── Text Block (PlayerHeader: "Player Parameters")
        │       ├── Vertical Box (PlayerParametersBox)
        │       ├── Spacer (Height: 10)
        │       ├── Text Block (AIHeader: "AI Parameters")
        │       ├── Vertical Box (AIParametersBox)
        │       ├── Spacer (Height: 10)
        │       ├── Text Block (NetworkHeader: "Network Parameters")
        │       └── Vertical Box (NetworkParametersBox)
        ├── Spacer (Height: 10)
        └── Horizontal Box (BottomControls)
            ├── Button (ApplyButton)
            │   └── Text: "Apply Changes"
            └── Button (CloseButton)
                └── Text: "Close"
```

### Variables
```
PlayerTuningData (PlayerTuningData Object Reference)
AIParametersData (AIParametersData Object Reference)
NetworkParamsData (NetworkParamsData Object Reference)

PlayerSliders (Array of WBP_ParameterSlider)
AISliders (Array of WBP_ParameterSlider)
NetworkSliders (Array of WBP_ParameterSlider)

ModifiedParameters (Map: String -> Float)
```

### Event Construct
```
Nodes:
1. Load DataAssets:
   - Load Object: "/Game/Data/DA_PlayerTuning"
   - Cast to PlayerTuningData
   - Set PlayerTuningData
   
   - Load Object: "/Game/Data/DA_AIParameters"
   - Cast to AIParametersData
   - Set AIParametersData
   
   - Load Object: "/Game/Data/DA_NetworkParams"
   - Cast to NetworkParamsData
   - Set NetworkParamsData

2. Call InitializeWithDataAssets(PlayerTuningData, AIParametersData, NetworkParamsData)

3. Call CreateParameterSliders()

4. Call PopulatePresetDropdown()
```

### Function: CreateParameterSliders
```
Nodes:
1. Create Player Parameter Sliders:
   For each parameter:
   - Create Widget: WBP_ParameterSlider
   - Call Initialize(Name, Min, Max, CurrentValue)
   - Bind OnValueChanged to HandleParameterChanged
   - Add to PlayerParametersBox
   - Add to PlayerSliders array

   Parameters:
   - MaxWalkSpeed (300, 1000, PlayerTuningData.MaxWalkSpeed)
   - MaxSprintSpeed (500, 1500, PlayerTuningData.MaxSprintSpeed)
   - Acceleration (500, 5000, PlayerTuningData.Acceleration)
   - Deceleration (1000, 8000, PlayerTuningData.Deceleration)
   - MaxStamina (50, 200, PlayerTuningData.MaxStamina)
   - SprintStaminaCost (5, 50, PlayerTuningData.SprintStaminaCost)
   - StaminaRegenRate (5, 30, PlayerTuningData.StaminaRegenRate)
   - TackleRange (50, 300, PlayerTuningData.TackleRange)
   - KickForce (500, 5000, PlayerTuningData.KickForce)

2. Create AI Parameter Sliders:
   For each parameter:
   - Create Widget: WBP_ParameterSlider
   - Call Initialize(Name, Min, Max, CurrentValue)
   - Bind OnValueChanged to HandleParameterChanged
   - Add to AIParametersBox
   - Add to AISliders array

   Parameters:
   - PerceptionRadius (500, 5000, AIParametersData.PerceptionRadius)
   - UpdateInterval (0.05, 1.0, AIParametersData.UpdateInterval)
   - Aggression (0, 1, AIParametersData.Aggression)
   - TacticalAwareness (0, 1, AIParametersData.TacticalAwareness)
   - MaxSpeed (300, 1000, AIParametersData.MaxSpeed)
   - AvoidanceRadius (50, 300, AIParametersData.AvoidanceRadius)

3. Create Network Parameter Sliders:
   For each parameter:
   - Create Widget: WBP_ParameterSlider
   - Call Initialize(Name, Min, Max, CurrentValue)
   - Bind OnValueChanged to HandleParameterChanged
   - Add to NetworkParametersBox
   - Add to NetworkSliders array

   Parameters:
   - CorrectionThreshold (1, 100, NetworkParamsData.CorrectionThreshold)
   - SmoothingSpeed (1, 50, NetworkParamsData.SmoothingSpeed)
   - InterpolationDelay (0.05, 0.5, NetworkParamsData.InterpolationDelay)
   - StateBufferSize (10, 100, NetworkParamsData.StateBufferSize)
   - SimulatedLatency (0, 500, NetworkParamsData.SimulatedLatency)
   - PacketLossPercentage (0, 50, NetworkParamsData.PacketLossPercentage)
```

### Function: HandleParameterChanged
```
Inputs: ParameterName (String), NewValue (Float)
Nodes:
1. Add to ModifiedParameters map: ParameterName -> NewValue
2. Call OnParameterChanged(ParameterName, NewValue) [C++ function]
3. Print String: "Parameter {ParameterName} changed to {NewValue}"
```

### Function: PopulatePresetDropdown
```
Nodes:
1. Call GetAvailablePresets() [C++ function]
2. Clear PresetDropdown options
3. For each preset name:
   - Add Option to PresetDropdown
4. Add default option: "Custom"
5. Set Selected Option to "Custom"
```

### Function: RefreshAllSliders
```
Nodes:
1. For each slider in PlayerSliders:
   - Get current value from PlayerTuningData
   - Call slider.UpdateValue(current value)

2. For each slider in AISliders:
   - Get current value from AIParametersData
   - Call slider.UpdateValue(current value)

3. For each slider in NetworkSliders:
   - Get current value from NetworkParamsData
   - Call slider.UpdateValue(current value)

4. Clear ModifiedParameters map
```

### Button Events

#### SavePresetButton OnClicked
```
Nodes:
1. Get Selected Option from PresetDropdown
2. Branch on Selected Option == "Custom":
   - True: Show input dialog for new preset name
   - False: Use selected option as preset name
3. Call SavePreset(PresetName) [C++ function]
4. Call PopulatePresetDropdown()
5. Print String: "Preset saved: {PresetName}"
```

#### LoadPresetButton OnClicked
```
Nodes:
1. Get Selected Option from PresetDropdown
2. Branch on Selected Option != "Custom":
   - True:
     - Call LoadPreset(Selected Option) [C++ function]
     - Call RefreshAllSliders()
     - Print String: "Preset loaded: {Selected Option}"
   - False:
     - Print String: "Please select a preset to load"
```

#### ResetButton OnClicked
```
Nodes:
1. Call ResetToDefaults() [C++ function]
2. Call RefreshAllSliders()
3. Print String: "Reset to default parameters"
```

#### ApplyButton OnClicked
```
Nodes:
1. Call ApplyChanges() [C++ function]
2. For each slider in all slider arrays:
   - Call slider.ResetToDefault() to clear modified indicators
3. Clear ModifiedParameters map
4. Print String: "Applied {count} parameter changes"
```

#### CloseButton OnClicked
```
Nodes:
1. Remove from Parent
2. Set Input Mode to Game Only
3. Show Mouse Cursor = false
```

---

## Part 3: Integration with Game

### In PlayerController Blueprint or C++

#### Add Variable
```
DesignerPanelWidget (WBP_DesignerPanel Widget Reference)
```

#### Add Input Action
```
Action Name: ToggleDesignerPanel
Key: F2
```

#### ToggleDesignerPanel Event
```
Nodes:
1. Branch on DesignerPanelWidget Is Valid:
   - True (Panel exists):
     - Remove DesignerPanelWidget from Parent
     - Set DesignerPanelWidget = null
     - Set Input Mode to Game Only
     - Show Mouse Cursor = false
   
   - False (Panel doesn't exist):
     - Create Widget: WBP_DesignerPanel
     - Set DesignerPanelWidget = Created Widget
     - Add to Viewport (Z-Order: 100)
     - Set Input Mode to UI Only
     - Show Mouse Cursor = true
```

---

## Styling Tips

### Colors
- Background: (0, 0, 0, 0.8) - Semi-transparent black
- Headers: (1, 1, 0, 1) - Yellow
- Normal Text: (1, 1, 1, 1) - White
- Modified Indicator: (1, 1, 0, 1) - Yellow
- Buttons: (0.2, 0.2, 0.2, 1) - Dark gray
- Button Hover: (0.3, 0.3, 0.3, 1) - Light gray

### Fonts
- Title: 24pt, Bold
- Headers: 16pt, Bold
- Parameters: 12pt, Regular
- Values: 12pt, Monospace

### Spacing
- Padding: 10px
- Spacer Height: 10px
- Slider Width: 300px
- Text Width: 200px

### Animations (Optional)
- Fade in when opening (0.2s)
- Fade out when closing (0.2s)
- Highlight modified parameters with pulse animation

---

## Testing Checklist

- [ ] Panel opens with F2 key
- [ ] All parameters are displayed with correct values
- [ ] Sliders update values in real-time
- [ ] Text boxes accept manual input
- [ ] Modified indicators appear when values change
- [ ] Save preset creates new preset
- [ ] Load preset restores saved values
- [ ] Reset to defaults restores original values
- [ ] Apply changes updates game behavior immediately
- [ ] Close button hides panel and restores game input
- [ ] Panel is draggable (if implemented)
- [ ] Tooltips show parameter descriptions (if implemented)

---

## Advanced Features (Optional)

### Search/Filter
Add a search box that filters parameters by name:
```
1. Add Editable Text Box above parameter scroll box
2. On Text Changed:
   - For each slider widget:
     - If parameter name contains search text:
       - Set Visibility = Visible
     - Else:
       - Set Visibility = Collapsed
```

### Collapsible Categories
Make each category collapsible:
```
1. Replace category headers with buttons
2. Add arrow icon (▼ / ►)
3. On button clicked:
   - Toggle category box visibility
   - Rotate arrow icon
```

### Parameter Comparison
Show difference from default:
```
1. Add Text Block next to value showing delta
2. Format: "+50" or "-25"
3. Color code: Green for positive, Red for negative
```

### Export/Import Presets
Save presets to JSON files:
```
1. Add Export button
2. Use File Dialog to select save location
3. Serialize preset data to JSON
4. Write to file

1. Add Import button
2. Use File Dialog to select file
3. Read JSON file
4. Deserialize and create preset
```
