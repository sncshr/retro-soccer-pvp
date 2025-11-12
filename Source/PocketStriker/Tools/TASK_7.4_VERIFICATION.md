# Task 7.4 Verification Guide

## How to Test Network Path Visualization

### Prerequisites
- Compile the project successfully
- Launch the game with debug HUD enabled
- Ensure `bShowDebugInfo = true` on the HUD

### Test 1: Input Buffer Visualization (2D HUD)

**Expected Behavior**:
1. Look at the "=== INPUT BUFFER ===" section on the HUD
2. Move the character (WASD keys)
3. You should see:
   - Buffered input count increasing
   - Recent inputs list showing last 5 inputs
   - Movement vectors displayed as (X, Y)
   - Action flags when pressing Sprint/Tackle/Kick/Pass
   - Buffer usage bar graph at the bottom

**Success Criteria**:
- ✅ Input count updates in real-time
- ✅ Recent inputs show correct sequence numbers
- ✅ Movement vectors match input direction
- ✅ Action flags appear when buttons pressed
- ✅ Bar graph fills proportionally to buffer usage
- ✅ Color changes: Green (<10), Yellow (10-30), Red (>30)

### Test 2: Predicted Path Visualization (3D)

**Expected Behavior**:
1. Enable network paths: `bShowNetworkPaths = true`
2. Move the character around
3. You should see in 3D space:
   - GREEN line trail following your movement
   - Small green spheres every 5 positions
   - Large green sphere at current position
   - "PREDICTED" label above character
   - Cyan arrow showing input direction
   - "Buffered: X inputs" text above character

**Success Criteria**:
- ✅ Green path trail follows character smoothly
- ✅ Path updates every frame
- ✅ Position markers visible along path
- ✅ Current position clearly marked
- ✅ Input direction arrow points correctly
- ✅ Labels are readable and positioned correctly

### Test 3: Reconciled Path Visualization (3D)

**Expected Behavior**:
1. Enable network simulation with latency:
   - Use NetworkDebugger to set simulated latency (100ms)
2. Move the character rapidly
3. When server corrections occur, you should see:
   - ORANGE/RED line trail showing reconciled path
   - Red sphere at correction point
   - Red arrow from correction point to current position
   - "Correction: X cm" label showing magnitude
   - "X.XXs ago" label showing time since correction
   - "RECONCILED (Total: X)" status label

**Success Criteria**:
- ✅ Orange path appears after corrections
- ✅ Correction sphere appears at correction point
- ✅ Arrow shows correction direction and magnitude
- ✅ Magnitude label shows distance in cm
- ✅ Time label updates and fades after 2 seconds
- ✅ Total corrections counter increments

### Test 4: Hitbox Visualization (3D)

**Expected Behavior**:
1. Enable hitboxes: `bShowHitboxes = true`
2. Look at your character and nearby actors
3. You should see:
   - **Player Character**:
     - Cyan box (actor bounds)
     - Magenta capsule (collision capsule)
     - Yellow box (mesh bounds)
     - White sphere and axes (root location)
     - Dimension labels (R: radius, H: height)
   - **Nearby Actors**:
     - Red boxes for AI actors
     - Green boxes for ball actors
     - Blue boxes for other actors

**Success Criteria**:
- ✅ Multiple overlapping shapes visible on player
- ✅ Capsule dimensions match character size
- ✅ Coordinate axes show forward/right/up directions
- ✅ Dimension labels are readable
- ✅ Nearby actors show colored bounds
- ✅ Actors beyond 2000cm are not drawn (performance)

### Test 5: Toggle Controls

**Expected Behavior**:
1. Toggle `bShowNetworkPaths` to false
   - Predicted and reconciled paths should disappear
2. Toggle `bShowHitboxes` to false
   - All collision visualization should disappear
3. Toggle `bShowDebugInfo` to false
   - Entire HUD should disappear

**Success Criteria**:
- ✅ Toggles work immediately
- ✅ No errors when toggling
- ✅ Performance improves when disabled

### Test 6: Network Correction Scenario

**Simulated Network Test**:
1. Set simulated latency to 100ms
2. Set packet loss to 5%
3. Move character in circles rapidly
4. Observe:
   - Green predicted path ahead of character
   - Orange reconciled path when corrections occur
   - Red correction markers appearing periodically
   - Correction magnitude varying based on movement

**Success Criteria**:
- ✅ Predicted path shows client-side movement
- ✅ Corrections visible when prediction diverges
- ✅ Correction magnitude correlates with movement speed
- ✅ System handles packet loss gracefully
- ✅ No crashes or visual artifacts

### Test 7: Multi-Actor Scenario

**Expected Behavior**:
1. Spawn AI actors nearby
2. Spawn ball actor
3. Move around the scene
4. Observe:
   - Player hitboxes in cyan/magenta/yellow
   - AI hitboxes in red
   - Ball hitbox in green
   - All actors within 2000cm visible
   - Distant actors culled

**Success Criteria**:
- ✅ All nearby actors show hitboxes
- ✅ Color coding correct for each actor type
- ✅ No performance issues with multiple actors
- ✅ Culling works at 2000cm distance

## Performance Verification

### Frame Rate Test
1. Enable all visualizations
2. Move around with multiple actors
3. Monitor FPS counter
4. Expected: Minimal impact (<5% frame time increase)

### Memory Test
1. Run for extended period (5+ minutes)
2. Monitor memory usage
3. Expected: No memory leaks, stable usage

## Common Issues and Solutions

### Issue: No paths visible
- **Solution**: Check `bShowNetworkPaths = true`
- **Solution**: Verify NetworkPrediction component exists on pawn
- **Solution**: Move character to generate path data

### Issue: No corrections visible
- **Solution**: Enable network simulation with latency
- **Solution**: Move rapidly to trigger corrections
- **Solution**: Check NetworkReconciler component exists

### Issue: Hitboxes not showing
- **Solution**: Check `bShowHitboxes = true`
- **Solution**: Verify actors have collision components
- **Solution**: Check actors are within 2000cm range

### Issue: Labels unreadable
- **Solution**: Adjust camera distance
- **Solution**: Check text scale settings
- **Solution**: Verify world lighting

## Console Commands (if implemented)

```
// Toggle debug HUD
ShowDebug HUD

// Toggle network paths
r.ShowNetworkPaths 1/0

// Toggle hitboxes
r.ShowHitboxes 1/0

// Set simulated latency
net.SimulateLatency 100

// Set packet loss
net.SimulatePacketLoss 5
```

## Expected Visual Output

### Normal Movement (No Corrections)
- Green predicted path following character
- No orange reconciled path
- No red correction markers
- Input buffer showing 1-5 inputs

### With Network Corrections
- Green predicted path (client prediction)
- Orange reconciled path (after correction)
- Red correction markers at divergence points
- Input buffer showing 10-30 inputs (higher latency)

### Collision Debugging
- Multiple colored shapes overlapping on character
- Nearby actors with colored bounds
- Clear visual distinction between actor types

## Sign-Off Checklist

- [ ] All 4 sub-tasks implemented
- [ ] DrawInputBuffer shows detailed input data
- [ ] DrawPredictedPath visualizes client prediction
- [ ] DrawReconciledPath shows server corrections
- [ ] DrawHitboxes displays collision bounds
- [ ] No compilation errors
- [ ] No runtime errors
- [ ] Performance acceptable
- [ ] Visual clarity good
- [ ] Documentation complete

## Task Status: ✅ COMPLETE

All network path visualization features have been implemented and are ready for testing.
