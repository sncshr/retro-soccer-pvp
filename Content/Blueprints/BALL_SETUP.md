# Ball Physics and Interactions Setup Guide

This guide explains how to create and configure the ball blueprint with physics simulation and player interactions.

## Overview

The ball is a physics-enabled actor that supports:
- Realistic physics simulation (mass, friction, restitution)
- Possession system (player can control the ball)
- Kick action (apply impulse to ball)
- Tackle action (intercept and gain possession)
- Pass action (directed ball movement with aim assist)
- Visual feedback for possession state

## Step 1: Create Ball Blueprint

### Create Blueprint Class

1. In Content Browser, navigate to `Content/Blueprints/`
2. Right-click and select **Blueprint Class**
3. Search for and select **Ball** as the parent class (C++ class)
4. Name it `BP_Ball`
5. Open the blueprint

### Configure Components

The C++ base class already includes:
- **SphereComponent** (root, physics enabled)
- **MeshComponent** (visual representation)
- **PossessionEffect** (particle system for visual feedback)

#### Configure Sphere Component
1. Select **SphereComponent** in the Components panel
2. In Details panel, verify:
   - **Sphere Radius**: 15.0 (football size)
   - **Simulate Physics**: Checked
   - **Enable Gravity**: Checked
   - **Collision Preset**: PhysicsActor
   - **Generate Hit Events**: Checked

#### Configure Mesh Component
1. Select **MeshComponent**
2. Set:
   - **Static Mesh**: Select a sphere mesh (or import a football mesh)
   - **Scale**: (0.3, 0.3, 0.3) to match sphere size
   - **Collision**: No Collision (sphere handles collision)
   - **Material**: Assign a football material

#### Configure Possession Effect (Optional)
1. Select **PossessionEffect**
2. Set:
   - **Template**: Select a particle system (glow, sparkles, etc.)
   - **Auto Activate**: Unchecked (controlled by code)
   - **Scale**: Adjust to fit ball size

## Step 2: Configure Physics Properties

In the **Class Defaults** panel:

### Basic Physics
- **Ball Mass**: 0.45 (standard football mass in kg)
- **Ball Friction**: 0.5 (surface friction)
- **Ball Restitution**: 0.6 (bounciness, 0-1 range)
- **Ball Linear Damping**: 0.5 (velocity damping)
- **Ball Angular Damping**: 0.3 (rotation damping)

### Possession Properties
- **Possession Radius**: 100.0 (distance to gain possession)
- **Possession Min Velocity**: 50.0 (max speed to gain possession)

### Kick Properties
- **Max Kick Force**: 2000.0 (maximum impulse force)
- **Pass Aim Assist Radius**: 500.0 (aim assist range)

### Visual Feedback
- **Possession Highlight Color**: Green (0, 1, 0, 1)
- **Show Possession Effect**: Checked

## Step 3: Create Ball Material

### Create Material
1. In Content Browser, navigate to `Content/Materials/`
2. Right-click and select **Material**
3. Name it `M_Ball`
4. Open the material editor

### Configure Material
1. Add a **Constant3Vector** node for base color (white or football pattern)
2. Connect to **Base Color** input
3. Set **Metallic**: 0.0
4. Set **Roughness**: 0.7 (slightly rough surface)
5. Optional: Add a texture for football pattern
6. Click **Apply** and **Save**

### Assign Material
1. Open **BP_Ball**
2. Select **MeshComponent**
3. In Details, find **Materials**
4. Set **Element 0**: M_Ball

## Step 4: Test Ball Physics

### Place Ball in Level
1. Open **DemoLevel**
2. Drag **BP_Ball** from Content Browser into the level
3. Place it at a height (e.g., Z = 200) to test gravity
4. Click **Play** (Alt+P)

### Verify Physics
- [ ] Ball falls due to gravity
- [ ] Ball bounces when hitting the ground
- [ ] Ball rolls and comes to rest
- [ ] Ball has realistic physics behavior

### Adjust Physics if Needed
- Too bouncy: Reduce **Ball Restitution**
- Slides too much: Increase **Ball Friction**
- Stops too quickly: Reduce **Ball Linear Damping**
- Spins too much: Increase **Ball Angular Damping**

## Step 5: Implement Player Interactions

### Add Input Bindings (if not already done)
1. Go to **Project Settings > Input**
2. Verify Action Mappings exist:
   - **Kick**: Left Mouse Button
   - **Tackle**: Space
   - **Pass**: Right Mouse Button

### Implement Kick in Player Controller

In **BP_PlayerCharacter** or **PocketStrikerPlayerController**:

1. Open the Event Graph
2. Add **InputAction Kick** event
3. Get **Action System** component
4. Call **Execute Kick Action**:
   - **Instigator**: Self
   - **Direction**: Get player forward vector
   - **Force**: Get from PlayerTuning (KickForce)

Example Blueprint nodes:
```
InputAction Kick
  -> Get Action System
  -> Execute Kick Action
    - Instigator: Self
    - Direction: Get Actor Forward Vector
    - Force: 2000.0 (or from PlayerTuning->KickForce)
```

### Implement Tackle in Player Controller

1. Add **InputAction Tackle** event
2. Get **Action System** component
3. Call **Execute Tackle Action**:
   - **Instigator**: Self

Example Blueprint nodes:
```
InputAction Tackle
  -> Get Action System
  -> Execute Tackle Action
    - Instigator: Self
```

### Implement Pass in Player Controller

1. Add **InputAction Pass** event
2. Find nearest teammate (or use camera direction)
3. Get **Action System** component
4. Call **Execute Pass Action**:
   - **Instigator**: Self
   - **Target Actor**: Nearest teammate or direction

Example Blueprint nodes:
```
InputAction Pass
  -> Find Nearest Teammate (custom function)
  -> Get Action System
  -> Execute Pass Action
    - Instigator: Self
    - Target Actor: Teammate
```

## Step 6: Test Ball Interactions

### Test Kick
1. Place **BP_Ball** near player spawn
2. Play the level
3. Move player close to ball
4. Press **Left Mouse Button** to kick
5. Verify:
   - [ ] Ball receives impulse
   - [ ] Ball moves in player's forward direction
   - [ ] Kick force is appropriate

### Test Tackle
1. Place ball in level
2. Play and move player close to ball
3. Press **Space** to tackle
4. Verify:
   - [ ] Player gains possession when close enough
   - [ ] Ball follows player when possessed
   - [ ] Possession effect activates

### Test Pass
1. Place multiple AI characters in level
2. Gain possession of ball
3. Press **Right Mouse Button** to pass
4. Verify:
   - [ ] Ball passes toward nearest teammate
   - [ ] Aim assist helps accuracy
   - [ ] Pass force is appropriate

## Step 7: Add Visual Feedback

### Create Dynamic Material Instance

In **BP_Ball** Event Graph:

1. On **BeginPlay**:
   - Get **MeshComponent**
   - Create Dynamic Material Instance from M_Ball
   - Store in variable

2. Create custom event **Update Ball Color**:
   - Branch on **Is Possessed**
   - If possessed: Set material color to green
   - If not possessed: Set material color to white

3. Call **Update Ball Color** when possession changes

### Add Possession Particle Effect

1. Find or create a particle system for possession (glow, sparkles)
2. Assign to **PossessionEffect** component
3. The C++ code automatically activates/deactivates it

### Add Sound Effects (Optional)

1. Import kick sound effect
2. In **BP_Ball**, add **Audio Component**
3. On kick event, play kick sound
4. Adjust volume and attenuation settings

## Step 8: Network Replication

The ball is already configured for network replication in C++:
- **Replicates**: True
- **Replicate Movement**: True
- **Net Update Frequency**: 60 Hz
- **Possessing Actor**: Replicated variable

### Test Networked Ball

1. In editor, set **Number of Players**: 2
2. Enable **Run Dedicated Server**
3. Click **Play**
4. Test ball interactions from both clients
5. Verify:
   - [ ] Ball position syncs between clients
   - [ ] Possession state replicates
   - [ ] Kicks and passes work over network

## Troubleshooting

### Ball Not Falling
- Check **Simulate Physics** is enabled
- Verify **Enable Gravity** is checked
- Check collision is set to PhysicsActor

### Ball Passes Through Floor
- Verify floor has collision enabled
- Check ball collision preset
- Ensure sphere radius is appropriate

### Kick Not Working
- Check input bindings are configured
- Verify ActionSystem is initialized
- Check ball is within kick range (200 units)
- Look for errors in Output Log

### Possession Not Working
- Verify **Possession Radius** is large enough
- Check ball velocity is below **Possession Min Velocity**
- Ensure overlap events are enabled on sphere component
- Check character has collision enabled

### Ball Too Bouncy/Slides Too Much
- Adjust **Ball Restitution** (bounciness)
- Adjust **Ball Friction** (sliding)
- Modify **Ball Linear Damping** (velocity decay)
- Create custom Physics Material for fine control

### Visual Feedback Not Showing
- Check **Show Possession Effect** is enabled
- Verify particle system is assigned
- Check material is dynamic material instance
- Look for errors in Output Log

## Advanced Features

### Ball Trajectory Prediction

The ball includes a **Predict Position At Time** function for AI:

```cpp
FVector PredictedPosition = Ball->PredictPositionAtTime(1.0f); // 1 second ahead
```

Use this in AI controllers for interception logic.

### Custom Physics Material

For more control over ball physics:

1. Create **Physics Material** asset
2. Set friction, restitution, and density
3. Assign to ball's sphere component
4. Fine-tune values for desired behavior

### Ball Trail Effect

Add a trail effect for visual feedback:

1. Add **Particle System Component** to ball
2. Use a trail emitter
3. Attach to ball mesh
4. Enable when ball is moving fast

### Ball Possession UI

Show possession state in HUD:

1. In **PocketStrikerDebugHUD**
2. Get all balls in level
3. For each ball, check **Is Possessed**
4. Draw possession info on screen

## Ball Setup Checklist

- [ ] BP_Ball blueprint created
- [ ] Sphere component configured with physics
- [ ] Mesh component assigned with material
- [ ] Physics properties tuned (mass, friction, restitution)
- [ ] Possession properties configured
- [ ] Kick properties configured
- [ ] Visual feedback working (possession effect)
- [ ] Input bindings configured
- [ ] Kick action implemented and tested
- [ ] Tackle action implemented and tested
- [ ] Pass action implemented and tested
- [ ] Ball physics feel realistic
- [ ] Network replication working

## Performance Considerations

- **Physics Simulation**: Ball uses physics, which has CPU cost
- **Update Frequency**: 60 Hz network updates for smooth replication
- **Collision**: Simple sphere collision is efficient
- **Particle Effects**: Keep possession effects lightweight
- **Sound**: Use sound attenuation to reduce distant sounds

## Next Steps

After completing ball setup:
1. Test full gameplay loop with player and AI
2. Tune ball physics for desired feel
3. Add more visual and audio feedback
4. Implement advanced features (ball trail, UI, etc.)
5. Polish and optimize

## Additional Resources

- [Unreal Engine Physics](https://docs.unrealengine.com/en-US/InteractiveExperiences/Physics/)
- [Physics Materials](https://docs.unrealengine.com/en-US/InteractiveExperiences/Physics/PhysicalMaterials/)
- [Network Replication](https://docs.unrealengine.com/en-US/InteractiveExperiences/Networking/Actors/)
- [Particle Systems](https://docs.unrealengine.com/en-US/RenderingAndGraphics/ParticleSystems/)

