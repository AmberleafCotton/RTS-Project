# GathererModule System Documentation

## Overview

The GathererModule is a comprehensive resource gathering and depositing system for RTS units. It provides a modular, extensible architecture that separates gathering logic from depositing logic, allowing for flexible resource management behaviors.

## Architecture
d
```
GathererModule (Core Controller)
├── GatherMethod (Gathering Strategy)
│   ├── NormalGathering (Slot-based, timed gathering)
│   └── [Future: RadiusBasedGathering, QueueBasedGathering, etc.]
└── DepositMethod (Depositing Strategy)
    ├── InstantDeposit (0.2s delay, immediate resource transfer)
    └── NormalDeposit (Movement to building, then transfer)
```

## Core Components

### 1. GathererModule (Main Controller)

**Purpose**: Central orchestrator that manages the complete gather → deposit → gather cycle.

**Key Responsibilities**:
- Manages unit resource state (`CurrentResourceAmount`, `CurrentResourceType`)
- Coordinates between gathering and depositing methods
- Handles resource type conflicts (deposit before gathering different types)
- Maintains target resource reference
- Broadcasts events for UI/feedback systems

**Core Functions**:
- `ExecuteGathererModule(ARTS_Actor* InTargetResource)` - Entry point for gathering commands
- `ResourceGathered(int32 ResourceAmount, EResourceType ResourceType)` - Called when gathering completes
- `ResourceDeposited(int32 DepositedAmount, EResourceType ResourceType)` - Called when depositing completes

**State Management**:
```cpp
UPROPERTY() TWeakObjectPtr<ARTS_Actor> TargetResource;
UPROPERTY() int32 CurrentResourceAmount = 0;
UPROPERTY() EResourceType CurrentResourceType;
UPROPERTY() int32 MaxResourceStorage = 1;
```

**Events**:
- `FOnResourceGathered` - Broadcast when resources are gathered
- `FOnResourceDeposited` - Broadcast when resources are deposited
- `FOnGatheringProgress` - Broadcast during gathering progress

### 2. GatherMethod (Gathering Strategy)

**Purpose**: Abstract base class for different gathering behaviors. Handles movement, slot management, and gathering timing.

**Key Responsibilities**:
- Manages movement to resource locations
- Handles slot acquisition and release
- Controls gathering timing and progress
- Manages movement event bindings
- Provides virtual functions for customization

**Core Functions**:
- `Gather(ARTS_Actor* ResourceTarget)` - Main gathering entry point
- `StopGather()` - Stops current gathering operation
- `StartGathering()` - Begins the gathering process
- `TickGathering()` - Updates gathering progress
- `CompleteGathering()` - Finalizes gathering and harvests resources

**Virtual Functions (Override These)**:
- `Gather_Implementation(ARTS_Actor* ResourceTarget)` - Customize gathering behavior
- `StopGather_Implementation()` - Customize cleanup behavior
- `StartGathering()` - Customize gathering start logic
- `TickGathering()` - Customize gathering progress logic
- `CompleteGathering()` - Customize resource harvesting logic

**Infrastructure Functions**:
- `RequestSlot(ARTS_Actor* ResourceTarget, FVector& OutSlotLocation)` - Get slot from resource
- `ReleaseSlot()` - Release current slot
- `MoveToLocation(const FVector& TargetLocation)` - Move to specific location
- `BindMovementEvents()` / `UnbindMovementEvents()` - Manage movement callbacks

### 3. DepositMethod (Depositing Strategy)

**Purpose**: Abstract base class for different depositing behaviors. Handles movement to deposit locations and resource transfer.

**Key Responsibilities**:
- Manages movement to deposit buildings/locations
- Handles resource transfer to player resources
- Controls depositing timing and flow
- Manages movement event bindings
- Provides virtual functions for customization

**Core Functions**:
- `Deposit(ARTS_Actor* Target)` - Main depositing entry point
- `StopDeposit()` - Stops current depositing operation
- `CompleteDepositing()` - Finalizes depositing and transfers resources
- `GetDepositLocation()` - Gets deposit building/location

**Virtual Functions (Override These)**:
- `Deposit_Implementation(ARTS_Actor* Target)` - Customize depositing behavior
- `StopDeposit_Implementation()` - Customize cleanup behavior
- `CompleteDepositing()` - Customize resource transfer logic
- `GetDepositLocation()` - Customize deposit location logic

**Infrastructure Functions**:
- `MoveToLocation(const FVector& TargetLocation)` - Move to specific location
- `BindMovementEvents()` / `UnbindMovementEvents()` - Manage movement callbacks

## Current Implementations

### NormalGathering

**Behavior**: Slot-based gathering with timed resource harvesting.

**Flow**:
1. Request slot from target resource
2. Move to slot location
3. Start gathering timer
4. Update progress every 0.2s
5. Harvest resource when timer completes
6. Continue gathering or deposit if full

**Key Features**:
- Uses `SlotModule` for worker limiting
- Timer-based gathering with progress updates
- Automatic slot release on resource depletion
- Movement event binding/unbinding

**Configuration**:
- Gathering time from `GatherableModule->GatheringTime`
- 0.2s tick interval for progress updates
- 150 unit acceptance radius for movement

### InstantDeposit

**Behavior**: Quick deposit with 0.2s delay for visual feedback.

**Flow**:
1. Start 0.2s timer
2. Transfer resources to player when timer expires
3. Reset unit resource state
4. Continue gathering cycle

**Key Features**:
- Minimal delay for visual feedback
- Direct resource transfer to `PlayerResourcesModule`
- No movement required
- Simple timer-based completion

### NormalDeposit

**Behavior**: Movement-based deposit requiring travel to deposit building.

**Flow**:
1. Find deposit building (currently `RTS_PlayerStart`)
2. Move to building location
3. Transfer resources when movement completes
4. Reset unit resource state
5. Continue gathering cycle

**Key Features**:
- Building location detection
- Movement-based completion
- Automatic building finding
- Movement event handling

**Building Detection**:
- Searches for actors with class name containing "RTS_PlayerStart"
- Returns building location for movement
- Falls back to immediate completion if no building found

## Complete System Flow

### 1. Initial Gathering Command
```
Player clicks resource → ExecuteGathererModule(Resource)
```

### 2. Resource Type Check
```
if (CurrentResourceAmount > 0 && DifferentResourceType) {
    DepositMethod->Deposit(Resource); // Deposit first
    return;
}
```

### 3. Capacity Check
```
if (CurrentResourceAmount >= MaxResourceStorage) {
    DepositMethod->Deposit(Resource); // Deposit if full
} else {
    GatherMethod->Gather(Resource); // Gather if not full
}
```

### 4. Gathering Process (NormalGathering)
```
1. Request slot from resource
2. Move to slot location
3. Start gathering timer
4. Update progress (0.2s intervals)
5. Harvest resource when complete
6. Call ResourceGathered(Amount, Type)
```

### 5. Resource Gathered
```
1. Update CurrentResourceAmount and CurrentResourceType
2. Broadcast OnResourceGathered event
3. Check if full:
   if (CurrentResourceAmount >= MaxResourceStorage) {
       DepositMethod->Deposit(TargetResource); // Start depositing
   } else if (TargetResource.IsValid()) {
       GatherMethod->Gather(TargetResource); // Continue gathering
   }
```

### 6. Depositing Process (InstantDeposit)
```
1. Start 0.2s timer
2. When timer expires: CompleteDepositing()
3. Transfer resources to PlayerResourcesModule
4. Call ResourceDeposited(Amount, Type)
```

### 7. Resource Deposited
```
1. Reset CurrentResourceAmount = 0
2. Broadcast OnResourceDeposited event
3. Continue gathering if target exists:
   if (TargetResource.IsValid()) {
       ExecuteGathererModule(TargetResource); // Restart cycle
   }
```

## Creating New Methods

### Creating a New GatherMethod

1. **Create Header File**:
```cpp
#pragma once
#include "GatherMethod.h"
#include "YourNewGathering.generated.h"

UCLASS()
class DRAKTHYSPROJECT_API UYourNewGathering : public UGatherMethod
{
    GENERATED_BODY()

public:
    virtual void Gather_Implementation(ARTS_Actor* ResourceTarget) override;
    virtual void StopGather_Implementation() override;
    virtual void StartGathering() override;
    virtual void TickGathering() override;
    virtual void CompleteGathering() override;

private:
    // Add your custom properties
    float CustomGatheringTime = 1.0f;
};
```

2. **Implement Key Functions**:
```cpp
void UYourNewGathering::Gather_Implementation(ARTS_Actor* ResourceTarget)
{
    // Call base for module setup
    Super::Gather_Implementation(ResourceTarget);
    
    // Your custom gathering logic
    // Example: No slots needed, just move to resource
    FVector ResourceLocation = ResourceTarget->GetActorLocation();
    BindMovementEvents();
    MoveToLocation(ResourceLocation);
}

void UYourNewGathering::StartGathering()
{
    // Your custom gathering start
    CurrentGatheringTime = 0.f;
    RequiredGatheringTime = CustomGatheringTime;
    
    // Start timer
    GathererModule->GetWorld()->GetTimerManager().SetTimer(
        GatheringTimer, 
        this, 
        &UYourNewGathering::TickGathering, 
        0.2f, 
        true
    );
}

void UYourNewGathering::CompleteGathering()
{
    // Your custom completion logic
    // Don't forget to harvest the resource!
    bool bHarvested = false;
    int32 OutAmount = 0;
    EResourceType OutType;
    
    GatherableModule->HarvestResource(1, bHarvested, OutAmount, OutType);
    
    if (bHarvested) {
        GathererModule->ResourceGathered(OutAmount, OutType);
    }
}
```

### Creating a New DepositMethod

1. **Create Header File**:
```cpp
#pragma once
#include "DepositMethod.h"
#include "YourNewDeposit.generated.h"

UCLASS()
class DRAKTHYSPROJECT_API UYourNewDeposit : public UDepositMethod
{
    GENERATED_BODY()

public:
    virtual void Deposit_Implementation(ARTS_Actor* Target) override;
    virtual void StopDeposit_Implementation() override;
    virtual void CompleteDepositing() override;
    virtual FVector GetDepositLocation() override;

private:
    // Add your custom properties
    float CustomDepositTime = 0.5f;
};
```

2. **Implement Key Functions**:
```cpp
void UYourNewDeposit::Deposit_Implementation(ARTS_Actor* Target)
{
    // Your custom deposit logic
    // Example: Timer-based deposit
    if (GathererModule && GathererModule->GetWorld()) {
        GathererModule->GetWorld()->GetTimerManager().SetTimer(
            DepositTimer, 
            this, 
            &UYourNewDeposit::CompleteDepositing, 
            CustomDepositTime, 
            false
        );
    }
}

FVector UYourNewDeposit::GetDepositLocation()
{
    // Your custom location logic
    // Example: Find nearest building
    return FindNearestBuildingLocation();
}

void UYourNewDeposit::CompleteDepositing()
{
    // Your custom completion logic
    // Don't forget to transfer resources!
    if (!GathererModule || !GathererModule->Owner) return;
    
    UPlayerResourcesModule* PlayerResources = URTSModuleFunctionLibrary::GetPlayerResources(GathererModule->Owner);
    if (!PlayerResources) return;
    
    if (GathererModule->CurrentResourceAmount > 0) {
        PlayerResources->AddResource(GathererModule->CurrentResourceType, GathererModule->CurrentResourceAmount);
        GathererModule->ResourceDeposited(GathererModule->CurrentResourceAmount, GathererModule->CurrentResourceType);
    }
}
```

## Configuration and Setup

### Required Modules

**For Resources**:
- `GatherableModule` - Defines resource properties (type, amount, gathering time)
- `SlotModule` - Manages worker slots (for slot-based gathering)

**For Units**:
- `GathererModule` - Main gathering controller
- `TeamComponent` - For player ownership (required for resource access)

**For Player**:
- `PlayerResourcesModule` - Stores player resources

### Blueprint Setup

1. **Create Resource Actor**:
   - Add `GatherableModule` and `SlotModule`
   - Configure resource type, amount, and gathering time

2. **Create Unit Actor**:
   - Add `GathererModule` and `TeamComponent`
   - Set `GatherMethod` and `DepositMethod` instances
   - Configure `MaxResourceStorage`

3. **Create Player State**:
   - Add `PlayerResourcesModule`
   - Configure initial resources

### Event Binding

**For UI/Feedback**:
```cpp
// Bind to gathering progress
GathererModule->OnGatheringProgress.AddDynamic(this, &YourClass::OnGatheringProgress);

// Bind to resource events
GathererModule->OnResourceGathered.AddDynamic(this, &YourClass::OnResourceGathered);
GathererModule->OnResourceDeposited.AddDynamic(this, &YourClass::OnResourceDeposited);
```

## Best Practices

### 1. Resource Management
- Always check for null pointers before accessing modules
- Use `URTSModuleFunctionLibrary` for safe module access
- Handle resource depletion gracefully

### 2. Movement Management
- Always bind/unbind movement events when needed
- Clear timers in stop functions
- Handle movement failures gracefully

### 3. State Management
- Keep state consistent between gathering and depositing
- Reset state properly in stop functions
- Use weak pointers for external references

### 4. Error Handling
- Log warnings for missing components
- Provide fallback behaviors for edge cases
- Don't let errors break the gathering cycle

### 5. Performance
- Use timers sparingly (0.2s intervals are good)
- Cache frequently accessed components
- Avoid expensive operations in tick functions

## Troubleshooting

### Common Issues

1. **Unit not gathering**:
   - Check if `GatherableModule` and `SlotModule` are present on resource
   - Verify `GatherMethod` is properly initialized
   - Check if slots are available

2. **Unit not depositing**:
   - Check if `PlayerResourcesModule` is accessible
   - Verify `DepositMethod` is properly initialized
   - Check if deposit location is found

3. **Infinite loops**:
   - Verify `CurrentResourceAmount` is properly reset
   - Check if `TargetResource` becomes invalid
   - Ensure stop functions are called when needed

4. **Movement issues**:
   - Verify `AIController` and `PathFollowingComponent` are present
   - Check if movement events are properly bound/unbound
   - Verify acceptance radius settings

### Debug Tips

1. **Enable debug logging**:
   - Set `bDrawDebugPath = true` for movement visualization
   - Add `UE_LOG` statements for flow tracking

2. **Check module states**:
   - Verify all required modules are present
   - Check module initialization order
   - Validate module configurations

3. **Monitor events**:
   - Bind to all events for debugging
   - Log event parameters for state tracking
   - Verify event timing and frequency

## Future Enhancements

### Planned Features

1. **Advanced Gathering Methods**:
   - `RadiusBasedGathering` - No slots, random positions
   - `QueueBasedGathering` - Wait in line for slots
   - `FormationGathering` - Organized worker positions

2. **Advanced Deposit Methods**:
   - `BuildingSpecificDeposit` - Different buildings for different resources
   - `CaravanDeposit` - Group deposit with movement
   - `RemoteDeposit` - Deposit without returning to base

3. **System Improvements**:
   - Resource queuing system
   - Dynamic slot allocation
   - Performance optimizations
   - Advanced error recovery

### Extension Points

1. **Custom Resource Types**:
   - Extend `EResourceType` enum
   - Add resource-specific gathering logic
   - Implement resource-specific deposit locations

2. **Custom Building Types**:
   - Extend `GetDepositLocation()` for building types
   - Add building-specific deposit logic
   - Implement building capacity systems

3. **Custom Movement Behaviors**:
   - Override movement functions for custom pathfinding
   - Add formation movement
   - Implement obstacle avoidance

---

*This documentation covers the complete GathererModule system. For specific implementation details, refer to the individual class files and their inline documentation.*
