# Network Module Implementation

This module implements authoritative server networking with client-side prediction for Pocket Striker.

## Components

### NetworkTypes.h/cpp
- **FInputPacket**: Client-to-server input packet with sequence numbering and checksum validation
- **FStateUpdatePacket**: Server-to-client state update with acknowledged sequence
- **FPredictionState**: Client-side state history for reconciliation
- Implements serialization and validation for all packet types

### NetworkPrediction.h/cpp
- **UNetworkPrediction**: Client-side prediction component
- Buffers client inputs with sequence numbers
- Maintains state history for reconciliation
- Simulates movement locally before server confirmation
- Provides unacknowledged input retrieval for replay

### NetworkReconciler.h/cpp
- **UNetworkReconciler**: Server reconciliation component
- Processes server corrections and determines if reconciliation is needed
- Replays unacknowledged inputs after correction
- Implements visual smoothing for small corrections
- Tracks correction statistics for debugging

### NetworkInterpolation.h/cpp
- **UNetworkInterpolation**: Remote entity interpolation component
- Buffers recent server states for smooth rendering
- Interpolates position, rotation, and velocity
- Implements extrapolation for packet loss scenarios
- Configurable interpolation delay (default 100ms)

### NetworkGameState.h/cpp
- **ANetworkGameState**: Authoritative server game state manager
- Processes client inputs on server
- Validates inputs to prevent cheating
- Broadcasts state updates at fixed tick rate
- Tracks acknowledged sequences per client

### NetworkDebugger.h/cpp
- **UNetworkDebugger**: Network debugging and lag simulation
- Simulates configurable latency (inbound/outbound)
- Simulates packet loss percentage
- Tracks network statistics (RTT, packet loss, corrections)
- Provides real-time debug visualization

## Usage

### Client Setup
1. Add UNetworkPrediction component to player character
2. Add UNetworkReconciler component to player character
3. Buffer inputs in PlayerController and send to server via RPC
4. Process server corrections through reconciler

### Server Setup
1. Spawn ANetworkGameState actor
2. Process client inputs through NetworkGameState
3. Validate inputs to prevent cheating
4. Broadcast state updates at fixed rate

### Remote Players
1. Add UNetworkInterpolation component to remote player characters
2. Buffer server state updates
3. Interpolate position/rotation for smooth rendering

### Debug/Testing
1. Create UNetworkDebugger instance
2. Configure simulated latency and packet loss
3. Enable debug display for real-time metrics
4. Monitor corrections and network performance

## Key Features

- **Client-Side Prediction**: Eliminates perceived input lag
- **Server Reconciliation**: Corrects client state when divergence detected
- **Input Validation**: Prevents cheating through server-side validation
- **Interpolation**: Smooth remote player rendering with buffering
- **Lag Simulation**: Test network behavior with artificial latency/packet loss
- **Debug Visualization**: Real-time network metrics display

## Network Flow

1. Client captures input and assigns sequence number
2. Client predicts movement locally
3. Client sends input to server via RPC
4. Server validates and simulates input
5. Server broadcasts authoritative state with acknowledged sequence
6. Client receives state update
7. If error exceeds threshold, client reconciles by:
   - Applying server state
   - Replaying unacknowledged inputs
8. Remote clients interpolate received states for smooth rendering

## Performance Considerations

- Fixed timestep simulation for deterministic results
- Circular buffers with configurable size limits
- Efficient state history management
- Minimal correction overhead with threshold-based reconciliation
