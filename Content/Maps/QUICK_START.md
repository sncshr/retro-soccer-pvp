# Demo Level Quick Start

This is a condensed version of the level setup. For detailed instructions, see [README.md](README.md).

## Quick Setup Steps

### 1. Create Level
- **File > New Level > Empty Level**
- Save as `DemoLevel` in `Content/Maps/`

### 2. Add Lighting (Drag from Place Actors)
- **Directional Light**: Location (0, 0, 500), Rotation (-45, 0, 0)
- **Sky Light**: Default settings, enable Real Time Capture
- **Sky Atmosphere**: Default settings
- **Post Process Volume**: Enable Infinite Extent

### 3. Create Field (BSP Box)
- **Geometry > Box**: Location (0, 0, 0), Scale (500, 300, 1)
- Enable Collision
- Add 4 wall boxes around perimeter (Scale: 500x1x100 and 1x300x100)

### 4. Add Spawn Points
- **Player Start** x2-4: Corners of field at Z=100
  - (-2000, -1000, 100) and (2000, 1000, 100)
- **Target Point** x4-6: Distributed across field at Z=100
  - Tag with "AISpawn"

### 5. Configure NavMesh
- **Nav Mesh Bounds Volume**: Location (0, 0, 0), Scale (55, 35, 5)
- **Build > Build Paths** (Ctrl+Shift+P)
- Press **P** to visualize (should see green overlay)

### 6. Configure Level
- **World Settings**: Set Game Mode and Player Controller
- **Project Settings > Maps & Modes**: Set DemoLevel as default map

### 7. Test
- Press **Play** (Alt+P)
- Verify spawning and collision
- Press **P** to check NavMesh coverage

## Verification Checklist

- [ ] Level created and saved as DemoLevel.umap
- [ ] Lighting added (Directional, Sky, Atmosphere)
- [ ] Field created (50m x 30m with collision)
- [ ] 2-4 Player Start actors placed
- [ ] 4-6 Target Point actors placed and tagged
- [ ] Nav Mesh Bounds Volume added and scaled
- [ ] Navigation built successfully (green overlay visible)
- [ ] Level plays without errors
- [ ] Default maps configured in Project Settings

## Common Issues

**NavMesh not showing**: Rebuild paths, check bounds volume size, enable collision on field

**Players falling through**: Enable collision on field geometry, adjust spawn Z-height

**No lighting**: Add Directional Light and Sky Light, rebuild lighting if needed

## Next Task

Once the level is set up, proceed to **Task 11.2: Import and configure animation assets**.

