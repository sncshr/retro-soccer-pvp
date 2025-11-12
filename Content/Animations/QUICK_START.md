# Animation Setup Quick Start

Condensed guide for setting up animations. See [README.md](README.md) for detailed instructions.

## Quick Steps

### 1. Download Animations from Mixamo
- Go to [mixamo.com](https://www.mixamo.com/)
- Download these animations (FBX for Unreal, 30fps, Without Skin):
  - Idle, Walking, Running, Fast Run, Kick, Slide Tackle
  - Optional: Turn Left, Turn Right, Jog, Stop

### 2. Import to Unreal
- Content Browser: `Content/Animations/`
- Right-click > **Import**
- Select all FBX files
- Import Settings:
  - First animation: Import Mesh + Skeleton
  - Others: Use existing skeleton, Import Animations only
- Click **Import All**

### 3. Configure Root Motion
For each animation:
- Open in Animation Editor
- Asset Details > Root Motion
- Enable **Enable Root Motion**
- Set **Root Motion Root Lock**: **Ref Pose**
- Save

### 4. Set Compression
For each animation:
- Asset Details > Compression
- Set **Compression Scheme**: **Automatic Compression**
- Click **Apply**

### 5. Create Motion Database
- Content/Data/ > Right-click > **Data Asset**
- Select **MotionDatabase** class
- Name: `DA_MotionDatabase`
- Add all animation sequences to the array
- Set Sample Rate: **30 Hz**
- Save

### 6. Preprocess Database
Run preprocessor (one of these methods):
- Console: `MotionMatching.PreprocessDatabase /Game/Data/DA_MotionDatabase`
- Or create Editor Utility Widget with preprocess button
- Wait for completion (1-2 minutes)

### 7. Create Blendspace
- Content/Animations/ > Right-click > **Blend Space**
- Name: `BS_Locomotion`
- Horizontal Axis: Direction (-180 to 180, 8 divisions)
- Vertical Axis: Speed (0 to 900, 3 divisions)
- Add animations at grid points:
  - Speed 0: Idle
  - Speed 300: Walk (8 directions)
  - Speed 600: Run (8 directions)
  - Speed 900: Sprint (8 directions)
- Save

## Verification Checklist

- [ ] 6+ animations imported successfully
- [ ] Root motion enabled on all animations
- [ ] Compression applied
- [ ] DA_MotionDatabase created and populated
- [ ] Database preprocessed (Feature Count > 0)
- [ ] BS_Locomotion blendspace created
- [ ] Blendspace has animations at key grid points
- [ ] All animations play correctly in editor

## Common Issues

**Import fails**: Check FBX format, ensure skeleton is valid

**Root motion not working**: Verify root bone exists, check Enable Root Motion is checked

**Preprocessing fails**: Check all animations use same skeleton, verify memory available

**Blendspace blending poor**: Increase interpolation time, add more intermediate animations

## Next Task

Once animations are set up, proceed to **Task 11.3: Create player and AI character blueprints**.

