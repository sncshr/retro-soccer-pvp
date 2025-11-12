# Animation Assets Setup Guide

This guide explains how to import, configure, and set up animation assets for the Pocket Striker motion matching system.

## Overview

The animation system requires 6-10 animation clips covering basic movement and actions. These animations will be processed by the motion matching preprocessor to create an indexed motion database.

## Required Animation Clips

### Essential Animations (Minimum 6)
1. **Idle** - Standing still, neutral pose
2. **Walk** - Forward walking cycle
3. **Run** - Forward running cycle
4. **Sprint** - Fast running cycle
5. **Kick** - Ball kicking action
6. **Tackle** - Sliding tackle or defensive action

### Recommended Additional Animations (4 more)
7. **Turn_Left** - 90-degree left turn while moving
8. **Turn_Right** - 90-degree right turn while moving
9. **Jog** - Medium-speed running (between walk and run)
10. **Stop** - Deceleration from run to idle

## Animation Sources

### Option 1: Mixamo (Free, Recommended for Prototyping)
1. Go to [mixamo.com](https://www.mixamo.com/)
2. Sign in with Adobe account (free)
3. Select a character (e.g., "Y Bot" for consistency)
4. Search and download these animations:
   - "Idle"
   - "Walking"
   - "Running"
   - "Fast Run" (for sprint)
   - "Kick"
   - "Slide Tackle" or "Tackle"
   - "Turn Left" and "Turn Right"
5. Download settings:
   - Format: **FBX for Unreal (.fbx)**
   - Skin: **Without Skin** (if using same character)
   - Frame Rate: **30 fps**
   - Keyframe Reduction: **None** (for motion matching quality)

### Option 2: Unreal Marketplace
- Search for "Football" or "Soccer" animation packs
- Look for packs with locomotion and action animations
- Ensure animations include root motion data

### Option 3: Custom Animations
- Create animations in Blender, Maya, or other 3D software
- Export as FBX with proper skeletal hierarchy
- Ensure consistent skeleton across all animations

## Step 1: Import Animation Sequences

### Import Settings
1. In Content Browser, navigate to `Content/Animations/`
2. Right-click and select **Import to /Game/Animations/**
3. Select all FBX animation files
4. In the FBX Import Options dialog, configure:

**Mesh Settings:**
- Skeletal Mesh: **Import** (first time) or **Use Existing** (subsequent imports)
- Import Mesh: Check only for first animation, uncheck for others
- Skeleton: Select existing skeleton or create new

**Animation Settings:**
- Import Animations: **Checked**
- Animation Length: **Exported Time**
- Import Custom Attribute: **Checked**
- Delete Existing Morph Target Curves: **Unchecked**

**Transform Settings:**
- Import Translation: (0, 0, 0)
- Import Rotation: (0, 0, 0) or (-90, 0, 0) if needed
- Import Uniform Scale: 1.0

5. Click **Import All**

### Verify Import
- Check that all animation sequences appear in Content/Animations/
- Open each animation in the Animation Editor
- Verify animations play correctly
- Check for any import warnings or errors

## Step 2: Configure Root Motion

Root motion is essential for motion matching to work correctly with character movement.

### Enable Root Motion
For each animation sequence:
1. Open the animation in the Animation Editor
2. In the **Asset Details** panel, find **Root Motion**
3. Enable **Enable Root Motion**
4. Set **Root Motion Root Lock**: 
   - **Ref Pose** for most animations
   - **Anim First Frame** for looping animations
5. Set **Force Root Lock**: **Unchecked** (usually)

### Configure Root Motion for Specific Animations

**Locomotion (Walk, Run, Sprint, Jog):**
- Enable Root Motion: **Yes**
- Root Motion Root Lock: **Ref Pose**
- These animations should move the character forward

**Turns (Turn_Left, Turn_Right):**
- Enable Root Motion: **Yes**
- Root Motion Root Lock: **Ref Pose**
- These should rotate the character

**Actions (Kick, Tackle):**
- Enable Root Motion: **Yes** (if animation has forward movement)
- Root Motion Root Lock: **Ref Pose**
- Actions may have both translation and rotation

**Idle:**
- Enable Root Motion: **No** (or Yes with minimal movement)
- Character should stay in place

### Verify Root Motion
1. In Animation Editor, enable **Show > Bones > Root Motion**
2. Play the animation and observe the root bone movement
3. The character should move naturally with the animation

## Step 3: Set Up Animation Compression

Optimize animations for performance while maintaining quality for motion matching.

### Compression Settings
For each animation sequence:
1. Open in Animation Editor
2. Go to **Asset Details > Compression**
3. Set **Compression Scheme**: 
   - **Automatic Compression** (recommended)
   - Or **Least Destructive** for highest quality
4. Set **Translation Compression Format**: **Automatic**
5. Set **Rotation Compression Format**: **Automatic**
6. Set **Scale Compression Format**: **Automatic**
7. Click **Apply** and verify animation quality

### Quality Check
- Play animation after compression
- Check for any visual artifacts or jittering
- If quality is poor, try "Least Destructive" compression
- Balance file size vs. quality based on needs

## Step 4: Create Motion Database Asset

The Motion Database is a custom asset that stores preprocessed animation data for motion matching.

### Create the Asset
1. In Content Browser, navigate to `Content/Data/`
2. Right-click and select **Miscellaneous > Data Asset**
3. Select **MotionDatabase** as the class
4. Name it `DA_MotionDatabase`
5. Open the asset

### Configure Motion Database
1. In the **Animation Sequences** array, add all imported animations:
   - Click **+** to add array elements
   - Assign each animation sequence
2. Set **Feature Extraction Settings**:
   - Sample Rate: **30 Hz** (matches animation frame rate)
   - Velocity Weight: **1.0**
   - Facing Weight: **0.5**
   - Joint Position Weight: **0.3**
3. Save the asset

### Run Preprocessor
The motion matching preprocessor must be run to generate the indexed database:

**Option A: Editor Utility Widget (Recommended)**
1. Create an Editor Utility Widget for the preprocessor
2. Add a button that calls `UMotionMatchingPreprocessor::PreprocessDatabase()`
3. Run the widget to process the database

**Option B: Console Command**
1. In the editor, open the Output Log
2. Type: `MotionMatching.PreprocessDatabase /Game/Data/DA_MotionDatabase`
3. Wait for processing to complete (may take 1-2 minutes)

**Option C: C++ Code**
```cpp
// In a custom editor module or utility
UMotionDatabase* Database = LoadObject<UMotionDatabase>(nullptr, TEXT("/Game/Data/DA_MotionDatabase"));
UMotionMatchingPreprocessor* Preprocessor = NewObject<UMotionMatchingPreprocessor>();
Preprocessor->PreprocessDatabase(Database);
```

### Verify Preprocessing
1. Open DA_MotionDatabase
2. Check that **Feature Count** is > 0
3. Verify **Index Built** is true
4. Check Output Log for any preprocessing errors

## Step 5: Create Blendspace Fallbacks

Blendspaces provide fallback animation when motion matching is unavailable or performance is insufficient.

### Create 2D Blendspace
1. In Content Browser, navigate to `Content/Animations/`
2. Right-click and select **Animation > Blend Space**
3. Select the skeleton used by your animations
4. Name it `BS_Locomotion`

### Configure Blendspace Axes
1. Open BS_Locomotion
2. Set **Horizontal Axis**:
   - Name: "Direction"
   - Minimum: -180.0
   - Maximum: 180.0
   - Grid Divisions: 8
3. Set **Vertical Axis**:
   - Name: "Speed"
   - Minimum: 0.0
   - Maximum: 900.0 (max sprint speed)
   - Grid Divisions: 3

### Add Animation Samples
Place animations at appropriate grid positions:

**Speed = 0 (Idle):**
- Direction 0: Idle animation

**Speed = 300 (Walk):**
- Direction 0: Walk Forward
- Direction 45: Walk Forward-Right (or blend)
- Direction 90: Walk Right (or blend)
- Direction 135: Walk Back-Right (or blend)
- Direction 180: Walk Backward (or blend)
- Direction -135: Walk Back-Left (or blend)
- Direction -90: Walk Left (or blend)
- Direction -45: Walk Forward-Left (or blend)

**Speed = 600 (Run):**
- Direction 0: Run Forward
- (Repeat directional pattern with run animations)

**Speed = 900 (Sprint):**
- Direction 0: Sprint Forward
- (Repeat directional pattern with sprint animations)

### Blendspace Settings
1. Enable **Interpolation Time**: 0.2 seconds
2. Set **Interpolation Type**: **Linear**
3. Enable **Target Weight Interpolation Speed Per Sec**: 5.0
4. Save the blendspace

## Step 6: Test Animations in Animation Editor

### Preview Individual Animations
1. Open each animation sequence
2. Click Play in the viewport
3. Verify:
   - Animation plays smoothly
   - Root motion moves character correctly
   - No visual artifacts or popping
   - Frame rate is consistent

### Preview Blendspace
1. Open BS_Locomotion
2. Move the preview point around the grid
3. Verify smooth blending between animations
4. Test all corners and edges of the blendspace

### Test Motion Database (Requires Animation Blueprint)
- Will be tested in Task 11.3 when creating the Animation Blueprint

## Animation Asset Checklist

- [ ] 6-10 animation sequences imported
- [ ] All animations use the same skeleton
- [ ] Root motion enabled and configured appropriately
- [ ] Compression settings applied
- [ ] DA_MotionDatabase created in Content/Data/
- [ ] All animations added to Motion Database
- [ ] Motion Database preprocessed successfully
- [ ] BS_Locomotion blendspace created
- [ ] Blendspace configured with 8-directional movement
- [ ] All animations tested in Animation Editor

## Troubleshooting

### Animations Not Importing
- Check FBX file format (should be FBX 2020 or earlier)
- Verify skeleton hierarchy is correct
- Ensure "Import Animations" is checked in import settings
- Check for error messages in Output Log

### Root Motion Not Working
- Verify "Enable Root Motion" is checked
- Check that root bone is named correctly (usually "root" or "Root")
- Ensure animation has actual root bone movement
- Try different Root Motion Root Lock settings

### Motion Database Preprocessing Fails
- Check that all animation sequences are valid
- Verify animations use the same skeleton
- Check Output Log for specific error messages
- Ensure sufficient memory for preprocessing

### Blendspace Blending Looks Bad
- Adjust interpolation time (increase for smoother blending)
- Check that animations at grid points are compatible
- Verify animation speeds match the axis values
- Consider adding more intermediate animations

### Animations Look Jittery
- Reduce compression (use "Least Destructive")
- Check original animation quality
- Verify frame rate is consistent (30 fps)
- Ensure no keyframe reduction was applied during import

## Performance Considerations

- **Animation Count**: More animations = better motion matching but longer preprocessing
- **Compression**: Balance quality vs. memory usage
- **Blendspace Complexity**: More grid divisions = smoother blending but more memory
- **Root Motion**: Essential for motion matching but adds slight overhead

## Next Steps

After completing animation setup:
1. Create Animation Blueprint (Task 11.3)
2. Integrate motion matching node
3. Wire up blendspace fallback
4. Test with player and AI characters

## Additional Resources

- [Unreal Engine Animation Documentation](https://docs.unrealengine.com/en-US/AnimatingObjects/SkeletalMeshAnimation/)
- [Root Motion Guide](https://docs.unrealengine.com/en-US/AnimatingObjects/SkeletalMeshAnimation/RootMotion/)
- [Blend Spaces](https://docs.unrealengine.com/en-US/AnimatingObjects/SkeletalMeshAnimation/Blendspaces/)
- [Mixamo](https://www.mixamo.com/)

