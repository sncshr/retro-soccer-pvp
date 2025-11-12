# Demo Level Setup Guide

This guide provides step-by-step instructions for creating the Pocket Striker demo level with field, NavMesh, and spawn points.

## Overview

The demo level is a simple football field (50m x 30m) with player and AI spawn points, configured for networked gameplay and AI navigation.

## Step 1: Create New Level

1. In Unreal Editor, go to **File > New Level**
2. Select **Empty Level** template
3. Save the level as `DemoLevel` in `Content/Maps/` folder

## Step 2: Add Basic Lighting

### Directional Light (Sun)
1. In the **Place Actors** panel, search for "Directional Light"
2. Drag it into the level
3. Set the following properties:
   - Location: (0, 0, 500)
   - Rotation: (-45, 0, 0) for angled sunlight
   - Intensity: 10.0
   - Light Color: Slightly warm white (255, 250, 240)
   - Enable **Atmosphere Sun Light** for sky interaction

### Sky Light
1. Add a **Sky Light** actor to the level
2. Set properties:
   - Intensity: 1.0
   - Enable **Real Time Capture** for dynamic lighting

### Sky Atmosphere
1. Add a **Sky Atmosphere** actor for realistic sky rendering
2. Use default settings

### Post Process Volume
1. Add a **Post Process Volume** actor
2. Enable **Infinite Extent (Unbound)** in the details panel
3. Adjust exposure and color grading as desired

## Step 3: Create Football Field

### Option A: Using BSP Brushes (Quick Setup)

#### Field Floor
1. In **Place Actors**, select **Geometry > Box**
2. Drag a box into the level
3. Set properties:
   - Location: (0, 0, 0)
   - Scale: (500, 300, 1) for 50m x 30m x 0.1m field
   - Material: Create or assign a grass material
   - Enable **Collision**

#### Field Boundaries (Walls)
1. Add 4 box brushes for walls around the field:
   - North Wall: Location (0, 1500, 50), Scale (500, 1, 100)
   - South Wall: Location (0, -1500, 50), Scale (500, 1, 100)
   - East Wall: Location (2500, 0, 50), Scale (1, 300, 100)
   - West Wall: Location (-2500, 0, 50), Scale (1, 300, 100)
2. Set material to a simple colored material for visibility
3. Enable **Collision** on all walls

### Option B: Using Static Mesh (Recommended for Production)

1. Import or create a static mesh for the football field
2. Place it at Location (0, 0, 0)
3. Ensure collision is enabled (check **Collision Complexity** in mesh settings)
4. Scale to 50m x 30m dimensions

## Step 4: Add Player Spawn Points

1. In **Place Actors**, search for "Player Start"
2. Add 2-4 **Player Start** actors at these positions:
   - Player 1: Location (-2000, -1000, 100), Rotation (0, 0, 0)
   - Player 2: Location (2000, 1000, 100), Rotation (0, 180, 0)
   - Player 3 (optional): Location (-2000, 1000, 100), Rotation (0, 0, 0)
   - Player 4 (optional): Location (2000, -1000, 100), Rotation (0, 180, 0)
3. Adjust Z-height (100) to be slightly above the field surface

## Step 5: Add AI Spawn Points

1. In **Place Actors**, search for "Target Point"
2. Add 4-6 **Target Point** actors for AI spawning:
   - AI 1: Location (-1500, 0, 100), Rotation (0, 0, 0)
   - AI 2: Location (1500, 0, 100), Rotation (0, 180, 0)
   - AI 3: Location (-1000, -800, 100), Rotation (0, 45, 0)
   - AI 4: Location (1000, 800, 100), Rotation (0, 225, 0)
   - AI 5 (optional): Location (0, -1000, 100), Rotation (0, 90, 0)
   - AI 6 (optional): Location (0, 1000, 100), Rotation (0, 270, 0)
3. Tag these actors with "AISpawn" for easy identification in code

## Step 6: Configure Navigation Mesh

### Add Nav Mesh Bounds Volume
1. In **Place Actors**, search for "Nav Mesh Bounds Volume"
2. Drag it into the level
3. Set properties:
   - Location: (0, 0, 0)
   - Scale: (55, 35, 5) to cover entire field plus margin
4. The volume should encompass the entire playable area

### Configure Nav Mesh Settings
1. Go to **Project Settings > Navigation Mesh**
2. Verify these settings:
   - Agent Radius: 50.0 (character capsule radius)
   - Agent Height: 200.0 (character capsule height)
   - Agent Max Slope: 45.0
   - Agent Max Step Height: 50.0
3. Click **Apply** to save settings

### Build Navigation Mesh
1. In the main toolbar, click **Build > Build Paths** (or press Ctrl+Shift+P)
2. Wait for the build to complete
3. To visualize the NavMesh:
   - Press **P** key in the viewport to toggle NavMesh visualization
   - You should see a green overlay on walkable surfaces

### Verify NavMesh Coverage
1. Ensure the NavMesh covers the entire field
2. Check that there are no gaps or holes
3. Verify AI spawn points are on valid NavMesh locations (green areas)

## Step 7: Set Up Camera

### Option A: Default Player Camera
The player controller will automatically create a camera component. No additional setup needed.

### Option B: Fixed Camera (Optional)
1. Add a **Camera Actor** to the level
2. Position it above the field for an overview:
   - Location: (0, 0, 3000)
   - Rotation: (-60, 0, 0)
3. In the camera details, enable **Auto Activate for Player 0**

## Step 8: Configure Level Settings

### World Settings
1. Open **Window > World Settings**
2. Set the following:
   - **Game Mode Override**: Select your custom game mode (if created)
   - **Default Pawn Class**: Will be set by game mode
   - **Player Controller Class**: PocketStrikerPlayerController

### Level Blueprint (Optional)
1. Open **Blueprints > Open Level Blueprint**
2. Add logic to spawn AI characters at Target Points on BeginPlay:

```
Event BeginPlay
  -> Get All Actors of Class (TargetPoint)
  -> ForEachLoop
    -> Spawn Actor from Class (BP_AICharacter)
      -> Location: Target Point Location
      -> Rotation: Target Point Rotation
```

## Step 9: Test the Level

### Basic Verification
1. Click **Play** (Alt+P) to test in editor
2. Verify:
   - Player spawns at a Player Start location
   - Field is visible and has collision
   - Lighting looks appropriate
   - No errors in the Output Log

### NavMesh Verification
1. Press **P** to show NavMesh in play mode
2. Verify green NavMesh overlay covers the field
3. Check that AI can navigate (if spawned)

### Network Test (Optional)
1. In the Play dropdown, select **Number of Players: 2**
2. Enable **Run Dedicated Server**
3. Click Play to test networked spawning

## Step 10: Save and Set as Default Map

1. Save the level (**Ctrl+S**)
2. Go to **Project Settings > Maps & Modes**
3. Set **Editor Startup Map** to `DemoLevel`
4. Set **Game Default Map** to `DemoLevel`
5. Set **Server Default Map** to `DemoLevel`

## Troubleshooting

### NavMesh Not Appearing
- Ensure Nav Mesh Bounds Volume is large enough
- Check that the field has collision enabled
- Rebuild navigation (Build > Build Paths)
- Verify Navigation Mesh settings in Project Settings

### Players Falling Through Floor
- Check that field geometry has collision enabled
- Verify Player Start Z-height is above the floor
- Check collision settings on the field mesh/brush

### AI Not Spawning
- Verify Target Points are tagged correctly
- Check Level Blueprint spawn logic
- Ensure BP_AICharacter blueprint exists
- Check Output Log for spawn errors

### Lighting Too Dark/Bright
- Adjust Directional Light intensity
- Modify Sky Light intensity
- Add Post Process Volume for exposure control
- Rebuild lighting (Build > Build Lighting)

## Next Steps

After completing the demo level setup:
1. Import and configure animation assets (Task 11.2)
2. Create player and AI character blueprints (Task 11.3)
3. Implement ball physics and interactions (Task 11.4)

## Field Dimensions Reference

- Total Field Size: 50m x 30m (5000cm x 3000cm in Unreal units)
- Field Center: (0, 0, 0)
- Field Bounds: X: -2500 to 2500, Y: -1500 to 1500
- Wall Height: 100cm (1m)
- NavMesh Coverage: Entire field plus small margin

## Performance Considerations

- Keep geometry simple for optimal performance
- Use static lighting where possible (bake lightmaps)
- Limit dynamic lights to essential ones
- Use LODs for any detailed meshes
- Keep NavMesh resolution appropriate (not too fine)

