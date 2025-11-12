# DataAsset Setup Guide

This guide explains how to create the DataAsset instances for the Designer Panel.

## Creating DataAssets

### 1. Player Tuning DataAsset

1. In Content Browser, navigate to `Content/Data/`
2. Right-click and select **Miscellaneous > Data Asset**
3. Select `PlayerTuningData` as the class
4. Name it `DA_PlayerTuning`
5. Open and configure default values:
   - MaxWalkSpeed: 600.0
   - MaxSprintSpeed: 900.0
   - Acceleration: 2000.0
   - Deceleration: 4000.0
   - MaxStamina: 100.0
   - SprintStaminaCost: 20.0
   - StaminaRegenRate: 15.0
   - TackleRange: 150.0
   - KickForce: 2000.0

### 2. AI Parameters DataAsset

1. Right-click and select **Miscellaneous > Data Asset**
2. Select `AIParametersData` as the class
3. Name it `DA_AIParameters`
4. Open and configure default values:
   - PerceptionRadius: 2000.0
   - UpdateInterval: 0.2
   - Aggression: 0.7
   - TacticalAwareness: 0.8
   - MaxSpeed: 600.0
   - AvoidanceRadius: 100.0

### 3. Network Parameters DataAsset

1. Right-click and select **Miscellaneous > Data Asset**
2. Select `NetworkParamsData` as the class
3. Name it `DA_NetworkParams`
4. Open and configure default values:
   - CorrectionThreshold: 10.0
   - SmoothingSpeed: 10.0
   - InterpolationDelay: 0.1
   - StateBufferSize: 32
   - SimulatedLatency: 0.0
   - PacketLossPercentage: 0.0

## Using DataAssets in Game

### In PlayerController

```cpp
UPROPERTY(EditDefaultsOnly, Category = "Tuning")
UPlayerTuningData* PlayerTuning;

void APocketStrikerPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    if (PlayerTuning)
    {
        // Apply tuning data to movement component
        if (UPlayerMovementComponent* MovementComp = GetPawn()->FindComponentByClass<UPlayerMovementComponent>())
        {
            MovementComp->MaxWalkSpeed = PlayerTuning->MaxWalkSpeed;
            MovementComp->MaxSprintSpeed = PlayerTuning->MaxSprintSpeed;
            // ... apply other parameters
        }
    }
}
```

### In AI Controller

```cpp
UPROPERTY(EditDefaultsOnly, Category = "AI")
UAIParametersData* AIParameters;

void AAIControllerFootball::BeginPlay()
{
    Super::BeginPlay();
    
    if (AIParameters)
    {
        PerceptionRadius = AIParameters->PerceptionRadius;
        UpdateInterval = AIParameters->UpdateInterval;
        // ... apply other parameters
    }
}
```

### In Network Components

```cpp
UPROPERTY(EditDefaultsOnly, Category = "Network")
UNetworkParamsData* NetworkParams;

void UNetworkReconciler::Initialize()
{
    if (NetworkParams)
    {
        CorrectionThreshold = NetworkParams->CorrectionThreshold;
        SmoothingSpeed = NetworkParams->SmoothingSpeed;
        // ... apply other parameters
    }
}
```

## Hot-Reload Support

The DataAssets support hot-reload, meaning changes made through the Designer Panel will take effect immediately without restarting the game. This is achieved by:

1. The Designer Panel directly modifies the DataAsset properties
2. Game systems reference the DataAsset pointers
3. When parameters change, systems read the updated values on the next frame

## Preset System

Presets are stored in the Designer Panel widget and can be saved/loaded at runtime. To persist presets between sessions, you can:

1. Use Unreal's SaveGame system
2. Export presets to JSON files
3. Create DataAsset variants for different preset configurations

## Testing DataAssets

1. Create the three DataAssets as described above
2. Assign them to your PlayerController, AIController, and Network components
3. Launch the game and verify default values are applied
4. Open the Designer Panel and modify values
5. Verify changes take effect immediately
6. Save a preset and reload it to verify persistence
