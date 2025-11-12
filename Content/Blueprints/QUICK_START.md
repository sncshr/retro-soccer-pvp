# Character Blueprints Quick Start

Condensed guide for creating character blueprints. See [README.md](README.md) for detailed instructions.

## Quick Steps

### 1. Create BP_PlayerCharacter
- Content/Blueprints/ > Right-click > **Blueprint Class > Character**
- Name: `BP_PlayerCharacter`
- **Mesh**: Assign skeletal mesh, set Anim Class later
- **Class Settings**: Auto Possess Player = Player 0
- **Add Components**:
  - PlayerMovementComponent
  - PlayerStateMachine
  - NetworkPrediction
  - NetworkReconciler
- **Class Defaults**:
  - Replicates: Checked
  - Replicate Movement: Checked
  - Assign DA_PlayerTuning
  - Assign DA_NetworkParams
- Compile and Save

### 2. Create BP_AICharacter
- Content/Blueprints/ > Right-click > **Blueprint Class > Character**
- Name: `BP_AICharacter`
- **Mesh**: Same as player, set Anim Class later
- **Class Settings**: 
  - Auto Possess AI = Placed in World
  - AIController Class = AIControllerFootball
- **Add Components**:
  - AISteeringComponent
  - AIPerceptionComponent (configure sight)
- **Class Defaults**:
  - Assign DA_AIParameters
- Compile and Save

### 3. Create ABP_Character
- Content/Animations/ > Right-click > **Animation > Animation Blueprint**
- Select character skeleton
- Name: `ABP_Character`
- **AnimGraph**:
  - Add **AnimNode_MotionMatching** node
  - Connect to Output Pose
  - Set Motion Database: DA_MotionDatabase
  - Set Fallback Blendspace: BS_Locomotion
  - Enable Use Async Search
- **EventGraph**:
  - Get velocity and speed from pawn owner
  - Store in variables
- Compile and Save

### 4. Assign Animation Blueprint
- Open **BP_PlayerCharacter**
- Mesh component > Anim Class = ABP_Character
- Compile and Save
- Open **BP_AICharacter**
- Mesh component > Anim Class = ABP_Character
- Compile and Save

### 5. Create Game Mode
- Content/Blueprints/ > Right-click > **Blueprint Class > Game Mode Base**
- Name: `BP_PocketStrikerGameMode`
- **Class Defaults**:
  - Default Pawn Class = BP_PlayerCharacter
  - Player Controller Class = PocketStrikerPlayerController
  - HUD Class = PocketStrikerDebugHUD
- Compile and Save

### 6. Configure Input
- **Project Settings > Input**
- **Action Mappings**:
  - Sprint: Left Shift
  - Kick: Left Mouse Button
  - Tackle: Space
  - Pass: Right Mouse Button
- **Axis Mappings**:
  - MoveForward: W (1.0), S (-1.0)
  - MoveRight: D (1.0), A (-1.0)

### 7. Set Default Game Mode
- **Project Settings > Maps & Modes**
- Default GameMode = BP_PocketStrikerGameMode
- Or in level: **World Settings > GameMode Override**

### 8. Test
- Open DemoLevel
- Place BP_PlayerCharacter or ensure Player Start exists
- Place 2-3 BP_AICharacter instances
- Press **Play** (Alt+P)
- Verify spawning, movement, and animations

## Verification Checklist

- [ ] BP_PlayerCharacter created with all components
- [ ] BP_AICharacter created with AI controller
- [ ] ABP_Character animation blueprint created
- [ ] Motion matching node configured
- [ ] Animation blueprint assigned to both characters
- [ ] BP_PocketStrikerGameMode created
- [ ] Game mode set as default
- [ ] Input bindings configured
- [ ] Characters spawn and animate in level
- [ ] Player responds to input
- [ ] AI has controller assigned

## Common Issues

**Character not spawning**: Check Game Mode settings, verify Player Start in level

**No animation**: Assign ABP_Character to Mesh component, verify Motion Database

**AI not working**: Set AIController Class, check Auto Possess AI setting

**Components missing**: Compile C++ project, restart editor

**Input not working**: Configure input bindings, verify PlayerController assigned

## Next Task

Once characters are set up, proceed to **Task 11.4: Implement ball physics and interactions**.

