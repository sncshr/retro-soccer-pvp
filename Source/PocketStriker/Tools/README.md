# Tools Module - Debug and Visualization

Debug tools and visualization for the PocketStriker project.

## Components

### PocketStrikerDebugHUD
Main debug HUD showing real-time performance and gameplay metrics.

**Features:**
- FPS counter with color-coded indicators
- Frame timing (target: 16.7ms for 60 FPS)
- Network stats (RTT, packet loss, corrections)
- AI behavior states
- Motion matching performance
- 3D path visualization

**Usage:** Set as HUD class in GameMode. Toggle with `bShowDebugInfo`.

### DebugVisualizer
3D debug drawing for gameplay systems.

**Features:**
- AI target lines and perception radius
- NavMesh bounds
- Predicted paths (green) and reconciled paths (red, dashed)

**Usage:**
```cpp
UDebugVisualizer* Visualizer = NewObject<UDebugVisualizer>(this);
Visualizer->DrawPerceptionRadius(AILocation, 2000.0f);
Visualizer->DrawPredictedPath(PathPoints, FColor::Green);
```

Toggle with `bEnableVisualization`.

### DesignerPanel ✅ COMPLETE (Task 7.5)
Runtime parameter editor for tuning gameplay values.

**Features:**
- 21 DataAsset parameters exposed through UI
- Real-time adjustment with hot-reload
- Preset save/load
- Console commands

**Parameters:**
- Player (9): Movement, Stamina, Actions
- AI (6): Perception, Behavior, Movement
- Network (6): Prediction, Interpolation, Debug

**Console Commands:**
```
ToggleDesignerPanel
SaveDesignerPreset MyPreset
LoadDesignerPreset MyPreset
ListDesignerPresets
ResetDesignerParameters
SetDesignerParameter MaxWalkSpeed 750.0
```

**Quick Setup:** See `Content/UI/QUICK_START.md`

### DesignerPanelCommands
Console commands for Designer Panel.

**Commands:**
- `ToggleDesignerPanel` - Open/close panel
- `SaveDesignerPreset <name>` - Save current parameters
- `LoadDesignerPreset <name>` - Load preset
- `ListDesignerPresets` - Show all presets
- `ResetDesignerParameters` - Reset to defaults
- `SetDesignerParameter <name> <value>` - Set parameter
- `GetDesignerParameter <name>` - Get parameter value

## Integration

Debug tools integrate automatically:
- HUD draws when `bShowDebugInfo` is true
- AI debug called from `AAIControllerFootball::Tick()`
- Network paths visualized through prediction/reconciliation components
- Motion matching debug exposed through `UMotionMatcher`

## Performance

- Debug drawing uses short durations (0.1s default)
- Path history limited to 100 points
- Can be toggled at runtime
- Stripped in shipping builds

## Color Coding

**Performance:** Green (≥60 FPS), Yellow (30-60 FPS), Red (<30 FPS)  
**Network:** Green (<100ms RTT), Yellow (100-200ms), Red (>200ms)  
**Paths:** Green (predicted), Red (reconciled), Orange (corrections)  
**AI:** Yellow (perception), Magenta (targets), Cyan (ball trajectory)
