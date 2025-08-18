# 🎯 GatherMethod System

> **Strategy-based gathering behaviors for flexible resource collection**

The GatherMethod system implements the **Strategy Pattern** for resource gathering, allowing units to use different gathering behaviors based on their needs, the resource type, or gameplay requirements. Each method encapsulates a complete gathering strategy with its own logic for movement, timing, and resource harvesting.

## 🏗️ Architecture Overview

### Strategy Pattern Implementation

```
UGatherMethod (Abstract Base)
├── UGatherMethod_001 (NormalGathering)
│   └── Slot-based, timer-driven gathering
├── UGatherMethod_002 (RadiusGathering)  
│   └── Radius-based, no-slot gathering
└── [Your Custom Method]
    └── Custom gathering behavior
```

### Core Philosophy

- **🎯 Single Responsibility**: Each method handles one specific gathering strategy
- **🔧 Extensibility**: Easy to add new gathering behaviors without modifying existing code
Every  Gatherer can have different GatherMethod, which makes them unique.
- **🔄 Event-Driven**: Clean integration with the GathererModule event system

## 📦 Current Implementations

### 1. NormalGathering (GatherMethod_001)

**Display Name**: "Normal Gathering"  
**Tool Tip**: "Slot-based gathering with timed resource harvesting"

#### Behavior Overview
Slot-based gathering that requires workers to reserve positions at resources. Uses a timer-driven approach with progress updates and automatic slot management.

#### Key Features
- ✅ **Slot Management**: Requests and manages worker slots via `SlotModule`
- ✅ **Timer-Driven**: Uses configurable gathering time with progress updates
- ✅ **Stack-Based Storage**: Method-local storage policy with stack counting
- ✅ **Range Checking**: 10-unit acceptance radius for immediate gathering
- ✅ **Resource Type Handling**: Automatic deposit when switching resource types
- ✅ **Slot Cleanup**: Automatic slot release on resource depletion

#### Configuration
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gather Method")
int32 StacksStorageAmount = 1;  // Max stacks to carry

UPROPERTY(BlueprintReadWrite, Category = "Gather Method")
int32 CurrentGatheredStacks = 0;  // Current carried stacks
```

#### Flow Diagram
```
1. Gather() called
   ├── Check resource type compatibility
   ├── Check storage capacity
   └── Request slot from resource
   
2. Slot acquired
   ├── Check if in range (10 units)
   ├── If in range → StartGathering()
   └── If not in range → MoveToLocation(SlotLocation)
   
3. StartGathering()
   ├── Set RequiredGatheringTime from GatherableModule
   ├── Start 0.2s tick timer
   └── Broadcast OnGatheringProgress events
   
4. TickGathering()
   ├── Update CurrentGatheringTime
   ├── Broadcast progress
   └── Check if complete
   
5. CompleteGathering()
   ├── Harvest 1 stack from resource
   ├── Increment CurrentGatheredStacks
   ├── Call ResourceGathered(Amount, Type)
   └── Decide: continue or deposit
```

#### Use Cases
- **Standard RTS gathering** (Age of Empires style)
- **Limited worker positions** at resources
- **Organized gathering** with clear worker positions
- **Resource management** with stack-based storage

---

### 2. RadiusGathering (GatherMethod_002)

**Display Name**: "Radius Gathering"  
**Tool Tip**: "Radius-based gathering without slot requirements"

#### Behavior Overview
Radius-based gathering that allows unlimited workers to gather from resources without slot restrictions. Workers move to random positions within the resource radius.

#### Key Features
- ✅ **No Slot Requirements**: Unlimited workers can gather simultaneously
- ✅ **Random Positioning**: Workers find random positions within resource radius
- ✅ **Amount-Based Storage**: Configurable storage capacity in units
- ✅ **Harvest Power**: Configurable amount harvested per cycle
- ✅ **Flexible Timing**: Uses standard gathering time from resource
- ✅ **Resource Type Handling**: Automatic deposit when switching resource types

#### Configuration
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gather Method")
int32 HarvestPower = 1;  // Units harvested per cycle

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gather Method")
int32 StoragePower = 5;  // Max units before deposit

UPROPERTY(BlueprintReadWrite, Category = "Gather Method")
int32 CurrentStoredUnits = 0;  // Current stored units
```

#### Flow Diagram
```
1. Gather() called
   ├── Check resource type compatibility
   ├── Check storage capacity
   └── Get random position in resource radius
   
2. Position acquired
   ├── Check if in range
   ├── If in range → StartGathering()
   └── If not in range → MoveToLocation(RandomPosition)
   
3. StartGathering()
   ├── Set RequiredGatheringTime from GatherableModule
   ├── Start 0.2s tick timer
   └── Broadcast OnGatheringProgress events
   
4. TickGathering()
   ├── Update CurrentGatheringTime
   ├── Broadcast progress
   └── Check if complete
   
5. CompleteGathering()
   ├── Harvest HarvestPower units from resource
   ├── Increment CurrentStoredUnits
   ├── Call ResourceGathered(Amount, Type)
   └── Decide: continue or deposit
```

#### Use Cases
- **Unlimited worker resources**
- **Quick resource collection**
- **Simple gathering mechanics**

---

## 🔧 Creating Custom GatherMethods

### Step 1: Create Header File

```cpp
#pragma once
#include "GatherMethod.h"
#include "YourCustomGathering.generated.h"

UCLASS(DisplayName = "Your Custom Gathering", 
       ToolTip = "Custom gathering behavior with unique features")
class DRAKTHYSPROJECT_API UYourCustomGathering : public UGatherMethod
{
    GENERATED_BODY()

public:
    // Override the main gathering function
    virtual void Gather(ARTS_Actor* ResourceTarget) override;
    
    // Override cleanup function
    virtual void StopGather() override;
    
    // Override gathering location logic
    virtual bool GetGatheringLocation(FVector& OutLocation) override;
    
    // Override gathering lifecycle functions
    virtual void StartGathering() override;
    virtual void TickGathering() override;
    virtual void CompleteGathering() override;

private:
    // Add your custom properties
    UPROPERTY(EditAnywhere, Category = "Custom Gathering")
    float CustomGatheringTime = 1.0f;
    
    UPROPERTY(EditAnywhere, Category = "Custom Gathering")
    int32 CustomHarvestAmount = 2;
    
    UPROPERTY(EditAnywhere, Category = "Custom Gathering")
    float CustomRange = 200.0f;
};
```

### Step 2: Implement Core Functions

#### Gather() - Main Entry Point
```cpp
void UYourCustomGathering::Gather(ARTS_Actor* ResourceTarget)
{
    // Call base for module setup
    Super::Gather(ResourceTarget);
    
    // Your custom pre-gathering logic
    if (!ValidateGatheringConditions(ResourceTarget)) {
        return;
    }
    
    // Get gathering location using your custom logic
    FVector GatheringLocation;
    if (GetGatheringLocation(GatheringLocation)) {
        // Move to location or start gathering if already there
        float Distance = FVector::Dist(Owner->GetActorLocation(), GatheringLocation);
        if (Distance <= CustomRange) {
            StartGathering();
        } else {
            GathererModule->MoveToLocation(GatheringLocation);
        }
    }
}
```

#### GetGatheringLocation() - Custom Positioning Logic
```cpp
bool UYourCustomGathering::GetGatheringLocation(FVector& OutLocation)
{
    if (!CurrentGatheringTarget.IsValid()) {
        return false;
    }
    
    // Your custom positioning logic
    // Example: Find position based on formation
    OutLocation = CalculateFormationPosition();
    return true;
}
```

#### StartGathering() - Custom Gathering Start
```cpp
void UYourCustomGathering::StartGathering()
{
    // Set up your custom gathering parameters
    CurrentGatheringTime = 0.f;
    RequiredGatheringTime = CustomGatheringTime;
    
    // Start your custom timer
    if (GathererModule && GathererModule->GetWorld()) {
        GathererModule->GetWorld()->GetTimerManager().SetTimer(
            GatheringTimer, 
            this, 
            &UYourCustomGathering::TickGathering, 
            0.2f,  // Tick interval
            true   // Repeating
        );
    }
}
```

#### TickGathering() - Custom Progress Logic
```cpp
void UYourCustomGathering::TickGathering()
{
    // Update gathering progress
    CurrentGatheringTime += 0.2f;
    
    // Broadcast progress event
    if (GathererModule) {
        GathererModule->OnGatheringProgress.Broadcast(CurrentGatheringTime, RequiredGatheringTime);
    }
    
    // Check if gathering is complete
    if (CurrentGatheringTime >= RequiredGatheringTime) {
        CompleteGathering();
    }
}
```

#### CompleteGathering() - Custom Resource Harvesting
```cpp
void UYourCustomGathering::CompleteGathering()
{
    // Clear the timer
    if (GathererModule && GathererModule->GetWorld()) {
        GathererModule->GetWorld()->GetTimerManager().ClearTimer(GatheringTimer);
    }
    
    // Harvest resources using your custom logic
    bool bHarvested = false;
    int32 OutAmount = 0;
    EResourceType OutType;
    
    if (GatherableModule) {
        GatherableModule->HarvestResource(CustomHarvestAmount, bHarvested, OutAmount, OutType);
    }
    
    if (bHarvested) {
        // Update your custom storage tracking
        UpdateCustomStorage(OutAmount);
        
        // Notify the GathererModule
        GathererModule->ResourceGathered(OutAmount, OutType);
    }
}
```

#### StopGather() - Custom Cleanup
```cpp
void UYourCustomGathering::StopGather()
{
    // Clear timers
    if (GathererModule && GathererModule->GetWorld()) {
        GathererModule->GetWorld()->GetTimerManager().ClearTimer(GatheringTimer);
    }
    
    // Your custom cleanup logic
    CleanupCustomResources();
    
    // Call base cleanup
    Super::StopGather();
}
```

### Step 3: Virtual Function Reference

#### Core Virtual Functions

| Function | Purpose | When to Override |
|----------|---------|------------------|
| `Gather(ARTS_Actor*)` | Main gathering entry point | Always - implement your gathering logic |
| `StopGather()` | Cleanup when stopping | Always - clean up your resources |
| `GetGatheringLocation(FVector&)` | Get gathering position | If you need custom positioning |
| `StartGathering()` | Begin gathering process | If you need custom gathering start logic |
| `TickGathering()` | Update gathering progress | If you need custom progress logic |
| `CompleteGathering()` | Finish gathering and harvest | Always - implement your harvesting logic |

#### Infrastructure Functions (Inherited)

| Function | Purpose | Usage |
|----------|---------|-------|
| `InitializeGatherMethod(UGathererModule*)` | Setup method with module | Called automatically |
| `RequestSlot(ARTS_Actor*, FVector&)` | Get slot from resource | For slot-based methods |
| `ReleaseSlot()` | Release current slot | For slot-based methods |
| `MoveToLocation(FVector)` | Move to location | Use GathererModule->MoveToLocation() |

## 📚 Integration Examples

### Integration with UI Systems

```cpp
// Bind to gathering progress for UI updates
void AYourUnit::SetupGatheringUI()
{
    if (GathererModule && GathererModule->GatherMethod) {
        GathererModule->OnGatheringProgress.AddDynamic(this, &AYourUnit::OnGatheringProgress);
    }
}

void AYourUnit::OnGatheringProgress(float CurrentTime, float RequiredTime)
{
    float Progress = FMath::Clamp(CurrentTime / RequiredTime, 0.0f, 1.0f);
    UpdateGatheringProgressBar(Progress);
}
``

## 📖 Related Documentation

- **[Main GathererModule README](../README.md)** - Complete system overview
- **[DepositMethod README](../DepositMethod/README.md)** - Depositing strategy system
- **[System Architecture](../log/SYSTEM_ANALYSIS.md)** - Deep dive into system design
- **[Issue Tracking](../log/TODO.md)** - Current issues and improvements

---

*This documentation covers the complete GatherMethod system. For specific implementation details, refer to the individual class files and their inline documentation.*
