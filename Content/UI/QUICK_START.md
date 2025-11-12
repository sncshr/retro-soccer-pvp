# Designer Panel - Quick Start Guide

## 🚀 Quick Setup (5 Minutes)

### Step 1: Create DataAssets (2 min)
1. Open Unreal Editor
2. Navigate to `Content/Data/`
3. Right-click → **Miscellaneous → Data Asset**
4. Create three DataAssets:
   - `DA_PlayerTuning` (class: PlayerTuningData)
   - `DA_AIParameters` (class: AIParametersData)
   - `DA_NetworkParams` (class: NetworkParamsData)
5. Leave default values as-is

### Step 2: Create Parameter Slider Widget (1 min)
1. Navigate to `Content/UI/`
2. Right-click → **User Interface → Widget Blueprint**
3. Name it `WBP_ParameterSlider`
4. Add this structure:
   ```
   Horizontal Box
   ├── Text Block (Name: ParameterNameText, Width: 200px)
   ├── Slider (Name: ParameterSlider, Width: 300px)
   ├── Editable Text Box (Name: ParameterValueText, Width: 80px)
   └── Text Block (Name: ModifiedIndicator, Text: "*", Hidden)
   ```
5. Save and close

### Step 3: Create Designer Panel Widget (2 min)
1. Right-click → **User Interface → Widget Blueprint**
2. Name it `WBP_DesignerPanel`
3. **File → Reparent Blueprint** → Select `DesignerPanel`
4. Add this structure:
   ```
   Canvas Panel
   └── Vertical Box
       ├── Text Block (Title: "Designer Parameter Editor")
       ├── Horizontal Box (Preset controls)
       ├── Scroll Box
       │   └── Vertical Box (Parameter containers)
       └── Button (Apply Changes)
   ```
5. In Event Graph:
   - Event Construct → Call `InitializeWithDataAssets`
   - Load the three DataAssets created in Step 1
6. Save and close

### Step 4: Test It! (30 sec)
1. Press **Play** in editor
2. Press **~** to open console
3. Type: `ToggleDesignerPanel`
4. Panel should appear (or show instructions to create widgets)

## 📖 Full Documentation

For detailed implementation:
- **Setup Guide**: `Content/UI/README.md`
- **Blueprint Guide**: `Content/UI/WBP_DesignerPanel_Implementation.md`
- **DataAsset Guide**: `Content/Data/README.md`
- **System Overview**: `Source/PocketStriker/Tools/TASK_7.5_IMPLEMENTATION_GUIDE.md`

## 🎮 Console Commands

```
ToggleDesignerPanel              # Open/close panel
SaveDesignerPreset MyPreset      # Save current settings
LoadDesignerPreset MyPreset      # Load saved settings
ListDesignerPresets              # Show all presets
ResetDesignerParameters          # Reset to defaults
SetDesignerParameter MaxWalkSpeed 750.0  # Set specific value
```

## ⚡ Hot-Reload Test

1. Open panel (F2 or console command)
2. Change `MaxWalkSpeed` to 800
3. Click "Apply Changes"
4. Move character → Speed changes immediately!
5. No restart needed ✨

## 🐛 Troubleshooting

**Panel doesn't appear?**
- Create the Blueprint widgets (Steps 2-3)
- Check parent class is `DesignerPanel`
- Verify DataAssets are created

**Parameters don't update?**
- Click "Apply Changes" button
- Check DataAssets are loaded in Event Construct
- Verify game systems reference the DataAssets

**Need help?**
- See full guides in `Content/UI/README.md`
- Check `TASK_7.5_COMPLETE.md` for details

## 📊 Parameters Available

**Player** (9): MaxWalkSpeed, MaxSprintSpeed, Acceleration, Deceleration, MaxStamina, SprintStaminaCost, StaminaRegenRate, TackleRange, KickForce

**AI** (6): PerceptionRadius, UpdateInterval, Aggression, TacticalAwareness, MaxSpeed, AvoidanceRadius

**Network** (6): CorrectionThreshold, SmoothingSpeed, InterpolationDelay, StateBufferSize, SimulatedLatency, PacketLossPercentage

---

**Total Time**: ~5 minutes to basic functionality
**Difficulty**: Easy (mostly drag-and-drop in editor)
**Result**: Professional parameter tuning system with hot-reload! 🎉
