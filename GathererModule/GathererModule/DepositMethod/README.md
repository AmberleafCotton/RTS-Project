# 💰 DepositMethod System

> **Strategy-based depositing behaviors for flexible resource transfer**

The DepositMethod system implements the **Strategy Pattern** for resource depositing, allowing units to use different depositing behaviors based on their needs, the building type, or gameplay requirements. Each method encapsulates a complete depositing strategy with its own logic for movement, timing, and resource transfer.

## 🏗️ Architecture Overview

### Strategy Pattern Implementation

```
UDepositMethod (Abstract Base)
├── UInstantDeposit
│   └── Quick deposit with minimal delay
├── UNormalDeposit
│   └── Movement-based deposit to buildings
└── [Your Custom Method]
    └── Custom depositing behavior
```

### Core Philosophy

- **🎯 Single Responsibility**: Each method handles one specific depositing strategy
- **🔧 Extensibility**: Easy to add new depositing behaviors without modifying existing code
- **⚡ Polymorphism**: Different methods can be swapped at runtime
- **🔄 Event-Driven**: Clean integration with the GathererModule event system
- **🏢 Building-Aware**: Methods can target different deposit locations and building types

## 📦 Current Implementations

### 1. InstantDeposit

**Display Name**: "Instant Deposit"  
**Tool Tip**: "Quick deposit with minimal delay for visual feedback"

#### Behavior Overview
Quick deposit method that transfers resources immediately with a minimal delay for visual feedback. No movement required - perfect for fast-paced gameplay or when deposit buildings are not needed.

#### Key Features
- ✅ **No Movement Required**: Immediate resource transfer without travel
- ✅ **Minimal Delay**: 0.2s delay for visual feedback only
- ✅ **Simple Logic**: Direct transfer to PlayerResourcesModule
- ✅ **Fast Performance**: No pathfinding or movement calculations
- ✅ **Universal Compatibility**: Works with any resource type
- ✅ **Idempotent**: Safe to call multiple times

#### Configuration
```cpp
// No additional configuration needed
// Uses default 0.2s delay for visual feedback
```

#### Flow Diagram
```
1. Deposit() called
   ├── Start 0.2s timer for visual feedback
   └── Wait for timer completion
   
2. Timer expires
   ├── Call CompleteDepositing()
   ├── Transfer resources to PlayerResourcesModule
   ├── Reset unit resource state
   └── Broadcast OnResourceDeposited event
   
3. Continue gathering cycle
   ├── Call ResourceDeposited(Amount, Type)
   └── Return to gathering if target exists
```

#### Use Cases
- **Fast-paced RTS games** (Starcraft style)
- **Simple resource management** without buildings
- **Quick resource transfer** for immediate feedback
- **Performance-critical scenarios** where movement is not needed
- **Prototype or testing environments**

---

### 2. NormalDeposit

**Display Name**: "Normal Deposit"  
**Tool Tip**: "Movement-based deposit requiring travel to deposit building"

#### Behavior Overview
Movement-based deposit that requires units to travel to a deposit building before transferring resources. Provides realistic resource management with building-based economies.

#### Key Features
- ✅ **Building-Based**: Requires travel to deposit buildings
- ✅ **Automatic Building Detection**: Finds deposit buildings automatically
- ✅ **Movement Integration**: Uses centralized movement system
- ✅ **Range Checking**: Validates proximity to deposit location
- ✅ **Fallback Handling**: Graceful handling when no buildings found
- ✅ **Idempotent**: Safe to call multiple times

#### Configuration
```cpp
// No additional configuration needed
// Automatically detects deposit buildings
// Uses 150-unit acceptance radius for movement
```

#### Flow Diagram
```
1. Deposit() called
   ├── Get deposit location via GetDepositLocation()
   ├── Check if already at deposit location
   └── If not in range → MoveToLocation(DepositLocation)
   
2. Movement completed
   ├── Check if movement was successful
   ├── If successful → CompleteDepositing()
   └── If failed → Handle error gracefully
   
3. CompleteDepositing()
   ├── Transfer resources to PlayerResourcesModule
   ├── Reset unit resource state
   └── Broadcast OnResourceDeposited event
   
4. Continue gathering cycle
   ├── Call ResourceDeposited(Amount, Type)
   └── Return to gathering if target exists
```

#### Building Detection Logic
```cpp
FVector UNormalDeposit::GetDepositLocation()
{
    // Search for actors with class name containing "RTS_PlayerStart"
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARTS_Actor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors) {
        if (Actor->GetClass()->GetName().Contains("RTS_PlayerStart")) {
            return Actor->GetActorLocation();
        }
    }
    
    // Fallback: return unit's current location
    return GathererModule->Owner->GetActorLocation();
}
```

#### Use Cases
- **Traditional RTS games** (Age of Empires style)
- **Building-based economies** with deposit buildings
- **Realistic resource management** with travel time
- **Strategic gameplay** requiring building placement
- **Economy-focused RTS mechanics**

---

## 🔧 Creating Custom DepositMethods

### Step 1: Create Header File

```cpp
#pragma once
#include "DepositMethod.h"
#include "YourCustomDeposit.generated.h"

UCLASS(DisplayName = "Your Custom Deposit", 
       ToolTip = "Custom deposit behavior with unique features")
class DRAKTHYSPROJECT_API UYourCustomDeposit : public UDepositMethod
{
    GENERATED_BODY()

public:
    // Override the main deposit function
    virtual void Deposit() override;
    
    // Override cleanup function
    virtual void StopDeposit() override;
    
    // Override deposit completion logic
    virtual void CompleteDepositing() override;
    
    // Override deposit location logic
    virtual FVector GetDepositLocation() override;

private:
    // Add your custom properties
    UPROPERTY(EditAnywhere, Category = "Custom Deposit")
    float CustomDepositTime = 0.5f;
    
    UPROPERTY(EditAnywhere, Category = "Custom Deposit")
    TSubclassOf<AActor> TargetBuildingClass;
    
    UPROPERTY(EditAnywhere, Category = "Custom Deposit")
    float CustomAcceptanceRadius = 200.0f;
};
```

### Step 2: Implement Core Functions

#### Deposit() - Main Entry Point
```cpp
void UYourCustomDeposit::Deposit()
{
    // Your custom pre-deposit logic
    if (!ValidateDepositConditions()) {
        return;
    }
    
    // Get deposit location using your custom logic
    FVector DepositLocation = GetDepositLocation();
    
    // Check if already at deposit location
    float Distance = FVector::Dist(GathererModule->Owner->GetActorLocation(), DepositLocation);
    if (Distance <= CustomAcceptanceRadius) {
        // Already at deposit location, complete immediately
        CompleteDepositing();
    } else {
        // Move to deposit location
        GathererModule->MoveToLocation(DepositLocation);
    }
}
```

#### GetDepositLocation() - Custom Location Logic
```cpp
FVector UYourCustomDeposit::GetDepositLocation()
{
    // Your custom location logic
    // Example: Find specific building type
    if (TargetBuildingClass) {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), TargetBuildingClass, FoundActors);
        
        if (FoundActors.Num() > 0) {
            // Find nearest building
            AActor* NearestBuilding = FindNearestBuilding(FoundActors);
            return NearestBuilding->GetActorLocation();
        }
    }
    
    // Fallback: return unit's current location
    return GathererModule->Owner->GetActorLocation();
}
```

#### CompleteDepositing() - Custom Resource Transfer
```cpp
void UYourCustomDeposit::CompleteDepositing()
{
    // Your custom completion logic
    if (!GathererModule || !GathererModule->Owner) {
        return;
    }
    
    // Get player resources module
    UPlayerResourcesModule* PlayerResources = URTSModuleFunctionLibrary::GetPlayerResources(GathererModule->Owner);
    if (!PlayerResources) {
        return;
    }
    
    // Transfer resources with your custom logic
    if (GathererModule->CurrentResourceAmount > 0) {
        // Apply custom transfer logic (e.g., bonuses, fees, etc.)
        int32 TransferAmount = CalculateTransferAmount(GathererModule->CurrentResourceAmount);
        
        PlayerResources->AddResource(GathererModule->CurrentResourceType, TransferAmount);
        GathererModule->ResourceDeposited(TransferAmount, GathererModule->CurrentResourceType);
    }
}
```

#### StopDeposit() - Custom Cleanup
```cpp
void UYourCustomDeposit::StopDeposit()
{
    // Clear any timers
    if (GathererModule && GathererModule->GetWorld()) {
        GathererModule->GetWorld()->GetTimerManager().ClearTimer(DepositTimer);
    }
    
    // Your custom cleanup logic
    CleanupCustomResources();
    
    // Call base cleanup
    Super::StopDeposit();
}
```

### Step 3: Virtual Function Reference

#### Core Virtual Functions

| Function | Purpose | When to Override |
|----------|---------|------------------|
| `Deposit()` | Main deposit entry point | Always - implement your deposit logic |
| `StopDeposit()` | Cleanup when stopping | Always - clean up your resources |
| `CompleteDepositing()` | Finish deposit and transfer | Always - implement your transfer logic |
| `GetDepositLocation()` | Get deposit location | If you need custom location logic |

#### Infrastructure Functions (Inherited)

| Function | Purpose | Usage |
|----------|---------|-------|
| `InitializeDepositMethod(UGathererModule*)` | Setup method with module | Called automatically |
| `MoveToLocation(FVector)` | Move to location | Use GathererModule->MoveToLocation() |

### Step 4: Best Practices

#### 1. Resource Management
```cpp
// Always check for null pointers
if (!GathererModule || !GathererModule->Owner) {
    return;
}

// Use safe module access
UPlayerResourcesModule* PlayerResources = URTSModuleFunctionLibrary::GetPlayerResources(GathererModule->Owner);
if (!PlayerResources) {
    UE_LOG(LogTemp, Warning, TEXT("Player resources module not found"));
    return;
}
```

#### 2. Timer Management
```cpp
// Always clear timers in StopDeposit()
void UYourCustomDeposit::StopDeposit()
{
    if (GathererModule && GathererModule->GetWorld()) {
        GathererModule->GetWorld()->GetTimerManager().ClearTimer(DepositTimer);
    }
    Super::StopDeposit();
}
```

#### 3. Location Validation
```cpp
// Validate deposit locations
FVector UYourCustomDeposit::GetDepositLocation()
{
    FVector Location = CalculateCustomLocation();
    
    // Validate the location is accessible
    if (!IsLocationAccessible(Location)) {
        return GathererModule->Owner->GetActorLocation(); // Fallback
    }
    
    return Location;
}
```

#### 4. Error Handling
```cpp
// Handle deposit failures gracefully
void UYourCustomDeposit::CompleteDepositing()
{
    if (!CanCompleteDeposit()) {
        // Handle failure - maybe retry or use fallback
        HandleDepositFailure();
        return;
    }
    
    // Proceed with normal deposit logic
    TransferResources();
}
```

## 🎨 Method Configuration

### Blueprint Setup

1. **Create Method Instance**:
```cpp
// In your unit's constructor or initialization
GathererModule->DepositMethod = CreateDefaultSubobject<UYourCustomDeposit>(TEXT("CustomDepositMethod"));
```

2. **Configure Properties**:
```cpp
// Set custom properties
UYourCustomDeposit* CustomMethod = Cast<UYourCustomDeposit>(GathererModule->DepositMethod);
if (CustomMethod) {
    CustomMethod->CustomDepositTime = 1.0f;
    CustomMethod->TargetBuildingClass = AYourDepositBuilding::StaticClass();
    CustomMethod->CustomAcceptanceRadius = 150.0f;
}
```

### Runtime Method Switching

```cpp
// Switch methods at runtime
void SwitchToCustomDeposit()
{
    if (GathererModule) {
        // Stop current deposit
        GathererModule->StopGathererModule();
        
        // Create and set new method
        UYourCustomDeposit* NewMethod = NewObject<UYourCustomDeposit>(GathererModule);
        GathererModule->DepositMethod = NewMethod;
        
        // Resume with new method
        GathererModule->ExecuteGathererModule(CurrentTarget);
    }
}
```

## 🔍 Debugging and Testing

### Debug Visualization

```cpp
// Enable debug drawing
UPROPERTY(EditAnywhere, Category = "Debug")
bool bDrawDebugDeposit = false;

// Draw debug information
void UYourCustomDeposit::DrawDebugInfo()
{
    if (bDrawDebugDeposit) {
        FVector Location = GetDepositLocation();
        DrawDebugSphere(GetWorld(), Location, CustomAcceptanceRadius, 12, FColor::Blue, false, 0.1f);
    }
}
```

### Logging and Monitoring

```cpp
// Add debug logging
void UYourCustomDeposit::Deposit()
{
    UE_LOG(LogTemp, Log, TEXT("CustomDeposit: Starting deposit"));
    
    // Your deposit logic...
    
    UE_LOG(LogTemp, Log, TEXT("CustomDeposit: Deposit location: %s"), 
           *GetDepositLocation().ToString());
}
```

### Performance Monitoring

```cpp
// Monitor deposit performance
void UYourCustomDeposit::CompleteDepositing()
{
    double StartTime = FPlatformTime::Seconds();
    
    // Your completion logic...
    
    double EndTime = FPlatformTime::Seconds();
    if (EndTime - StartTime > 0.001f) {  // Log if taking more than 1ms
        UE_LOG(LogTemp, Warning, TEXT("CustomDeposit: Completion took %.3f ms"), 
               (EndTime - StartTime) * 1000.0);
    }
}
```

## 🚀 Advanced Features

### Building-Specific Deposits

```cpp
// Implement building-specific deposit logic
class UBuildingSpecificDeposit : public UDepositMethod
{
private:
    UPROPERTY(EditAnywhere, Category = "Building-Specific")
    TMap<EResourceType, TSubclassOf<AActor>> ResourceToBuildingMap;
    
public:
    virtual FVector GetDepositLocation() override
    {
        EResourceType ResourceType = GathererModule->CurrentResourceType;
        TSubclassOf<AActor> BuildingClass = ResourceToBuildingMap.FindRef(ResourceType);
        
        if (BuildingClass) {
            return FindBuildingOfType(BuildingClass);
        }
        
        return Super::GetDepositLocation();
    }
};
```

### Caravan Deposits

```cpp
// Implement caravan-style deposits
class UCaravanDeposit : public UDepositMethod
{
private:
    UPROPERTY(EditAnywhere, Category = "Caravan")
    int32 MinCaravanSize = 3;
    
    UPROPERTY(EditAnywhere, Category = "Caravan")
    float CaravanFormationRadius = 100.0f;
    
public:
    virtual void Deposit() override
    {
        // Check if enough units for caravan
        if (GetNearbyGatherers().Num() >= MinCaravanSize) {
            FormCaravanAndMove();
        } else {
            // Fall back to normal deposit
            Super::Deposit();
        }
    }
};
```

### Remote Deposits

```cpp
// Implement remote deposits without returning to base
class URemoteDeposit : public UDepositMethod
{
private:
    UPROPERTY(EditAnywhere, Category = "Remote")
    float RemoteDepositRange = 500.0f;
    
public:
    virtual void CompleteDepositing() override
    {
        // Transfer resources remotely without movement
        TransferResourcesRemotely();
        
        // Continue gathering immediately
        if (GathererModule->TargetResource.IsValid()) {
            GathererModule->ExecuteGathererModule(GathererModule->TargetResource.Get());
        }
    }
};
```

### Multi-Stage Deposits

```cpp
// Implement multi-stage deposit process
class UMultiStageDeposit : public UDepositMethod
{
private:
    UPROPERTY(EditAnywhere, Category = "Multi-Stage")
    TArray<EDepositStage> DepositStages;
    
    UPROPERTY(BlueprintReadWrite, Category = "Multi-Stage")
    int32 CurrentStage = 0;
    
public:
    virtual void CompleteDepositing() override
    {
        if (CurrentStage < DepositStages.Num()) {
            // Complete current stage
            CompleteStage(DepositStages[CurrentStage]);
            CurrentStage++;
            
            // Start next stage if available
            if (CurrentStage < DepositStages.Num()) {
                StartStage(DepositStages[CurrentStage]);
            } else {
                // All stages complete
                FinalizeDeposit();
            }
        }
    }
};
```

## 📚 Integration Examples

### Integration with Building Systems

```cpp
// Integrate with building management system
void UBuildingAwareDeposit::CompleteDepositing()
{
    // Find the deposit building
    AActor* DepositBuilding = FindDepositBuilding();
    if (DepositBuilding) {
        // Notify building of deposit
        UBuildingModule* BuildingModule = URTSModuleFunctionLibrary::GetBuildingModule(DepositBuilding);
        if (BuildingModule) {
            BuildingModule->OnResourceDeposited(GathererModule->CurrentResourceType, 
                                               GathererModule->CurrentResourceAmount);
        }
    }
    
    // Complete normal deposit
    Super::CompleteDepositing();
}
```

### Integration with Economy Systems

```cpp
// Integrate with economy management
void UEconomyAwareDeposit::CompleteDepositing()
{
    // Apply economy modifiers
    float EconomyMultiplier = GetEconomyMultiplier();
    int32 BaseAmount = GathererModule->CurrentResourceAmount;
    int32 FinalAmount = FMath::RoundToInt(BaseAmount * EconomyMultiplier);
    
    // Transfer with economy bonus
    UPlayerResourcesModule* PlayerResources = URTSModuleFunctionLibrary::GetPlayerResources(GathererModule->Owner);
    if (PlayerResources) {
        PlayerResources->AddResource(GathererModule->CurrentResourceType, FinalAmount);
        GathererModule->ResourceDeposited(FinalAmount, GathererModule->CurrentResourceType);
    }
}
```

### Integration with AI Systems

```cpp
// Integrate with AI decision making
void UAIOptimizedDeposit::GetDepositLocation()
{
    // Use AI to find optimal deposit location
    FVector OptimalLocation = AI_CalculateOptimalDepositLocation();
    
    // Consider factors like:
    // - Distance to current gathering location
    // - Building capacity and efficiency
    // - Enemy threat level
    // - Resource type priorities
    
    return OptimalLocation;
}
```

---

## 📖 Related Documentation

- **[Main GathererModule README](../README.md)** - Complete system overview
- **[GatherMethod README](../GatherMethod/README.md)** - Gathering strategy system
- **[System Architecture](../log/SYSTEM_ANALYSIS.md)** - Deep dive into system design
- **[Issue Tracking](../log/TODO.md)** - Current issues and improvements

---

*This documentation covers the complete DepositMethod system. For specific implementation details, refer to the individual class files and their inline documentation.*
