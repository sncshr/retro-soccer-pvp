# Character Blueprints Setup Guide

This guide explains how to create player and AI character blueprints with all required components for the Pocket Striker gameplay system.

## Overview

We need to create two main character blueprints:
1. **BP_PlayerCharacter** - Player-controlled character with input handling and network prediction
2. **BP_AICharacter** - AI-controlled character with perception and behavior systems

Both characters share the same animation system and skeletal mesh.

## Prerequisites

Before creating character blueprints:
- [ ] Demo level created (Task 11.1)
- [ ] Animations imported and configured (Task 11.2)
- [ ] Motion Database preprocessed
- [ ] Blendspace created
- [ ] C++ classes compiled successfully

## Part 1: Create Player Character Blueprint

### Step 1: Create Blueprint Class

1. In Content Browser, navigate to `Content/Blueprints/`
2. Right-click and select **Blueprint Class**
3. Select **Character** as the parent class
4. Name it `BP_PlayerCharacter`
5. Open the blueprint

### Step 2: Configure Character Components

#### Skeletal Mesh
1. Select **Mesh** component in the Components panel
2. In Details panel, set:
   - **Skeletal Mesh**: Select your imported character mesh
   - **Location**: (0, 0, -90) to align with capsule
   - **Rotation**: (0, 0, -90) if needed for forward direction
   - **Animation Mode**: Use Animation Blueprint
   - **Anim Class**: Will be set after creating ABP_Character

#### Capsule Component
1. Select **CapsuleComponent**
2. Set:
   - **Capsule Half Height**: 90.0
   - **Capsule Radius**: 40.0
   - **Collision Preset**: Pawn

### Step 3: Set Controller Class

1. In the **Class Defaults** (click Class Settings button)
2. Find **Pawn** section
3. Set **Auto Possess Player**: Player 0 (for single player) or Disabled (for multiplayer)
4. Find **Controller** section
5. Set **AIController Class**: None (player controlled)

### Step 4: Add Custom Components

#### Add PlayerMovementComponent
1. Click **Add Component** button
2. Search for "PlayerMovementComponent"
3. Add it to the blueprint
4. In Details panel:
   - Set **Max Walk Speed**: 600.0
   - Set **Max Acceleration**: 2000.0
   - Set **Braking Deceleration Walking**: 4000.0

#### Add PlayerStateMachine
1. Click **Add Component**
2. Search for "PlayerStateMachine"
3. Add it to the blueprint
4. This component manages player action states

#### Add NetworkPrediction
1. Click **Add Component**
2. Search for "NetworkPrediction"
3. Add it to the blueprint
4. This handles client-side prediction

#### Add NetworkReconciler
1. Click **Add Component**
2. Search for "NetworkReconciler"
3. Add it to the blueprint
4. This handles server corrections

### Step 5: Configure Replication

1. In **Class Defaults**, find **Replication** section
2. Enable **Replicates**: Checked
3. Enable **Replicate Movement**: Checked
4. Set **Net Update Frequency**: 60.0 (for responsive networking)

### Step 6: Assign DataAssets

1. In **Class Defaults**, find custom properties
2. Assign **Player Tuning Data**: DA_PlayerTuning
3. Assign **Network Params Data**: DA_NetworkParams

### Step 7: Compile and Save

1. Click **Compile** button
2. Fix any errors shown in the Compiler Results
3. Click **Save**

## Part 2: Create AI Character Blueprint

### Step 1: Create Blueprint Class

1. In Content Browser, navigate to `Content/Blueprints/`
2. Right-click and select **Blueprint Class**
3. Select **Character** as the parent class
4. Name it `BP_AICharacter`
5. Open the blueprint

### Step 2: Configure Character Components

#### Skeletal Mesh
1. Select **Mesh** component
2. Set the same skeletal mesh as player character
3. Set same location and rotation offsets
4. Set **Animation Mode**: Use Animation Blueprint
5. Set **Anim Class**: Will be set after creating ABP_Character

#### Capsule Component
Same settings as player character

### Step 3: Set AI Controller Class

1. In **Class Defaults**, find **Pawn** section
2. Set **Auto Possess AI**: Placed in World or Spawned
3. Set **AIController Class**: AIControllerFootball

### Step 4: Add AI Components

#### Add AISteeringComponent
1. Click **Add Component**
2. Search for "AISteeringComponent"
3. Add it to the blueprint
4. This handles AI movement and steering behaviors

#### Add AIPerceptionComponent (Unreal Built-in)
1. Click **Add Component**
2. Search for "AIPerception"
3. Add **AIPerceptionComponent**
4. Configure senses:
   - Add **AI Sight** config
   - Set **Sight Radius**: 2000.0
   - Set **Lose Sight Radius**: 2500.0
   - Set **Peripheral Vision Angle**: 90.0

### Step 5: Assign DataAssets

1. In **Class Defaults**, find custom properties
2. Assign **AI Parameters Data**: DA_AIParameters

### Step 6: Compile and Save

1. Click **Compile**
2. Fix any errors
3. Click **Save**

## Part 3: Create Animation Blueprint

### Step 1: Create Animation Blueprint

1. In Content Browser, navigate to `Content/Animations/`
2. Right-click and select **Animation > Animation Blueprint**
3. Select your character skeleton
4. Name it `ABP_Character`
5. Open the animation blueprint

### Step 2: Set Up Animation Graph

#### Add Motion Matching Node
1. In the **AnimGraph** tab
2. Right-click in the graph and search for "Motion Matching"
3. Add **AnimNode_MotionMatching** node
4. Connect it to **Output Pose**

#### Configure Motion Matching Node
1. Select the Motion Matching node
2. In Details panel:
   - Set **Motion Database**: DA_MotionDatabase
   - Set **Use Async Search**: True
   - Set **Fallback Blendspace**: BS_Locomotion

#### Add Blendspace Fallback (Optional)
If you want manual control over fallback:
1. Add a **Branch** node
2. Add a **Blend Poses by Bool** node
3. Connect Motion Matching to one input
4. Add **Blendspace Player** node with BS_Locomotion
5. Connect to other input
6. Use a boolean variable to switch between them

### Step 3: Set Up Event Graph

#### Get Character State
1. In the **EventGraph** tab
2. From **Event Blueprint Update Animation**:
   - Get Pawn Owner
   - Cast to Character
   - Get Velocity
   - Get Vector Length (for speed)
   - Store in variables: **Speed**, **Direction**

#### Update Motion Matching Query
1. Get reference to Motion Matching node
2. Call **Update Query Feature** with:
   - Current velocity
   - Current facing direction
   - Current action state (from PlayerStateMachine)

### Step 4: Create Variables

Add these variables for animation control:
- **Speed** (Float): Current movement speed
- **Direction** (Float): Movement direction angle
- **IsInAir** (Boolean): Whether character is jumping/falling
- **CurrentAction** (EPlayerAction): Current action being performed

### Step 5: Compile and Save

1. Click **Compile**
2. Fix any errors
3. Click **Save**

### Step 6: Assign to Characters

1. Open **BP_PlayerCharacter**
2. Select **Mesh** component
3. Set **Anim Class**: ABP_Character
4. Compile and save

5. Open **BP_AICharacter**
6. Select **Mesh** component
7. Set **Anim Class**: ABP_Character
8. Compile and save

## Part 4: Test Character Spawning

### Test in Demo Level

1. Open **DemoLevel**
2. Drag **BP_PlayerCharacter** into the level (or ensure Player Start is present)
3. Drag **BP_AICharacter** into the level at a few locations
4. Click **Play** (Alt+P)

### Verify Functionality

**Player Character:**
- [ ] Spawns at Player Start location
- [ ] Responds to WASD input
- [ ] Camera follows character
- [ ] Animation plays (idle when stationary)
- [ ] Collision works (doesn't fall through floor)

**AI Character:**
- [ ] Spawns at placed location
- [ ] Has AI controller assigned
- [ ] Animation plays
- [ ] Collision works

### Debug Visualization

Enable debug visualization to verify components:
1. Press **`** (tilde) to open console
2. Type: `showdebug ai` to see AI debug info
3. Type: `showdebug animation` to see animation debug
4. Type: `stat fps` to see performance

## Part 5: Configure Game Mode

### Create Game Mode Blueprint

1. In Content Browser, navigate to `Content/Blueprints/`
2. Right-click and select **Blueprint Class**
3. Select **Game Mode Base** as parent
4. Name it `BP_PocketStrikerGameMode`
5. Open the blueprint

### Configure Game Mode

1. In **Class Defaults**:
   - Set **Default Pawn Class**: BP_PlayerCharacter
   - Set **Player Controller Class**: PocketStrikerPlayerController
   - Set **HUD Class**: PocketStrikerDebugHUD (if using debug HUD)

### Set as Default Game Mode

1. Open **World Settings** in the level
2. Set **GameMode Override**: BP_PocketStrikerGameMode
3. Save the level

Or set globally:
1. Go to **Project Settings > Maps & Modes**
2. Set **Default GameMode**: BP_PocketStrikerGameMode

## Part 6: Advanced Configuration

### Add Input Bindings

1. Go to **Project Settings > Input**
2. Add Action Mappings:
   - **Sprint**: Left Shift
   - **Kick**: Left Mouse Button
   - **Tackle**: Space
   - **Pass**: Right Mouse Button
3. Add Axis Mappings:
   - **MoveForward**: W (1.0), S (-1.0)
   - **MoveRight**: D (1.0), A (-1.0)
   - **LookUp**: Mouse Y (1.0)
   - **LookRight**: Mouse X (1.0)

### Configure Camera

In **BP_PlayerCharacter**:
1. Add **Spring Arm** component
2. Set **Target Arm Length**: 400.0
3. Set **Location**: (0, 0, 60)
4. Enable **Use Pawn Control Rotation**
5. Add **Camera** component as child of Spring Arm
6. Set **Location**: (0, 0, 0)

### Network Optimization

For multiplayer:
1. In character blueprints, set **Net Update Frequency**: 60
2. Enable **Replicate Movement**
3. Set **Net Priority**: 3.0 (high priority for player characters)
4. Configure **Net Cull Distance Squared**: 15000000 (large view distance)

## Troubleshooting

### Character Not Spawning
- Check Player Start is in the level
- Verify Game Mode is set correctly
- Check Default Pawn Class in Game Mode
- Look for errors in Output Log

### Animation Not Playing
- Verify Anim Class is set on Mesh component
- Check that Motion Database is assigned
- Ensure animations are preprocessed
- Check for errors in Animation Blueprint

### AI Controller Not Assigned
- Verify AIController Class is set in BP_AICharacter
- Check Auto Possess AI is set to "Placed in World"
- Ensure AIControllerFootball class exists and compiles

### Components Not Found
- Verify C++ project compiled successfully
- Check that custom components are in the correct module
- Refresh the editor (File > Refresh Visual Studio Project)
- Restart Unreal Editor

### Movement Not Working
- Check input bindings are configured
- Verify PlayerController is assigned
- Check that Movement Component is added
- Enable input in PlayerController BeginPlay

### Network Prediction Issues
- Verify NetworkPrediction component is added
- Check that Replicates is enabled
- Ensure server/client roles are set correctly
- Test with simulated latency

## Blueprint Checklist

- [ ] BP_PlayerCharacter created and configured
- [ ] BP_AICharacter created and configured
- [ ] ABP_Character animation blueprint created
- [ ] Motion matching node added to anim graph
- [ ] All components added to characters
- [ ] DataAssets assigned
- [ ] Skeletal meshes assigned
- [ ] Animation blueprint assigned to meshes
- [ ] BP_PocketStrikerGameMode created
- [ ] Game mode set as default
- [ ] Input bindings configured
- [ ] Characters tested in demo level
- [ ] Animations playing correctly
- [ ] AI controller functioning

## Performance Considerations

- **LODs**: Set up LODs for skeletal meshes if needed
- **Tick Frequency**: Reduce tick rate for distant AI characters
- **Animation Update Rate**: Use Animation Update Rate Optimization
- **Culling**: Configure proper cull distances for AI characters
- **Bone Updates**: Limit bone updates when characters are off-screen

## Next Steps

After completing character blueprint setup:
1. Implement ball physics and interactions (Task 11.4)
2. Test gameplay with player and AI
3. Tune movement and animation parameters
4. Add visual effects and polish

## Additional Resources

- [Unreal Engine Character Setup](https://docs.unrealengine.com/en-US/InteractiveExperiences/Framework/Pawn/Character/)
- [Animation Blueprints](https://docs.unrealengine.com/en-US/AnimatingObjects/SkeletalMeshAnimation/AnimBlueprints/)
- [AI Controllers](https://docs.unrealengine.com/en-US/InteractiveExperiences/ArtificialIntelligence/AIControllers/)
- [Network Replication](https://docs.unrealengine.com/en-US/InteractiveExperiences/Networking/Actors/)

