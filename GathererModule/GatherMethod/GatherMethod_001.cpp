#include "GatherMethod_001.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Utilis/Libraries/RTSModuleFunctionLibrary.h"

void UGatherMethod_001::Gather(ARTS_Actor* TargetResource)
{
	Super::Gather(TargetResource);

	// Method 001Policy: if carrying different resource type than target, deposit first
	if (GathererModule->CurrentResourceAmount > 0 && GathererModule->CurrentResourceType != GatherableModule->ResourceType)
	{
		GathererModule->RequestDeposit();
		return;
	}

    // Method 001 Policy: If we recently deposited (module amount cleared), reset local stacks
    if (GathererModule->CurrentResourceAmount == 0 && CurrentGatheredStacks > 0)
    {
        CurrentGatheredStacks = 0;
    }

    // Method 001 Policy: if storage full by method-local stacks, deposit
    if (CurrentGatheredStacks >= StacksStorageAmount)
	{
		GathererModule->RequestDeposit();
		return;
	}

	// If everything is ok, then start getting GatheringLocation

	// Get gathering location using method-specific logic
	FVector GatheringLocation;
	if (GetGatheringLocation(GatheringLocation))
	{
		// If already close enough, start gathering immediately
		const float AcceptanceRadius = 25.0f;
		const FVector GathererLocation = GathererModule->Owner->GetActorLocation();
		if (FVector::DistSquared2D(GathererLocation, GatheringLocation) <= FMath::Square(AcceptanceRadius))
		{
			UE_LOG(LogTemp, Log, TEXT("UGatherMethod_001::Gather() - Already at gathering location; starting gathering. Loc=%s"), *GathererLocation.ToString());
			StartGathering();
		}
		else
		{
			// Otherwise move to the gathering location
			UE_LOG(LogTemp, Log, TEXT("UGatherMethod_001::Gather() - Moving to gathering location. From=%s To=%s Dist2D=%.2f"), *GathererLocation.ToString(), *GatheringLocation.ToString(), FVector::Dist2D(GathererLocation, GatheringLocation));
			GathererModule->MoveToLocation(GatheringLocation);
		}
	}
	else
	{
		// No valid gathering location found
		UE_LOG(LogTemp, Warning, TEXT("UGatherMethod_001::Gather() - No valid gathering location found"));
	}
}

void UGatherMethod_001::StartGathering()
{
	CurrentGatheringTime = 0.f;
	RequiredGatheringTime = GatherableModule->GatheringTime;

	GathererModule->GetWorld()->GetTimerManager().SetTimer(GatheringTimer, this, &UGatherMethod_001::TickGathering, 0.2f, true);
}

void UGatherMethod_001::TickGathering()
{
	CurrentGatheringTime += 0.2f;

	if (!GathererModule || !GatherableModule) return;

	GathererModule->OnGatheringProgress.Broadcast(CurrentGatheringTime, RequiredGatheringTime);

	if (CurrentGatheringTime >= RequiredGatheringTime)
	{
		CompleteGathering();
	}
}

void UGatherMethod_001::CompleteGathering()
{
	if (!GathererModule || !GatherableModule) return;

	// Clear timer
	if (GathererModule && GathererModule->GetWorld())
	{
		GathererModule->GetWorld()->GetTimerManager().ClearTimer(GatheringTimer);
	}
	
	// Reset progress immediately when gathering completes
	GathererModule->OnGatheringProgress.Broadcast(0.0f, 0.0f);

    bool bHarvested = false;
    int32 OutAmount = 0;
    EResourceType OutType;

    // Harvest a single stack worth of units
    GatherableModule->HarvestStack(1, bHarvested, OutType, OutAmount);

    if (bHarvested)
    {
        // Inform module of gather event (amount/type for UI and global state)
        GathererModule->ResourceGathered(OutAmount, OutType);
		// Increment method-local stacks
		CurrentGatheredStacks = FMath::Clamp(CurrentGatheredStacks + 1, 0, StacksStorageAmount);

        // Re-enter via single entrypoint so Gather() performs the next decision (deposit vs continue)
        if (CurrentGatheringTarget.IsValid())
        {
            GathererModule->ExecuteGathererModule(CurrentGatheringTarget.Get());
        }
    }
	else
	{
		GathererModule->OnGatheringProgress.Broadcast(0.0f, 0.0f);
		// Reset progress immediately when gathering completes
		
		//rEMEMBER ABOUT DELAY HERE
		//Find New Resource  with the same ResourceType.
		//GathererModule->FindNewResource();
	}
}

void UGatherMethod_001::StopGather()
{
	// Safely get the SlotModule and free up the slot if it exists
	if (SlotModule)
	{
		if (GathererModule && GathererModule->Owner)
		{
			SlotModule->FreeUpSlot(GathererModule->Owner);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UGatherMethod_001::StopGather() - GathererModule or Owner is null"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UGatherMethod_001::StopGather() - Failed to get SlotModule for target"));
	}
	Super::StopGather();
}

bool UGatherMethod_001::GetGatheringLocation(FVector& OutLocation)
{
	// This method uses slot-based gathering
	if (!CurrentGatheringTarget.IsValid())
	{
		OutLocation = FVector::ZeroVector;
		return false;
	}

	// Get SlotModule for the target resource
	SlotModule = URTSModuleFunctionLibrary::GetSlotModule(CurrentGatheringTarget.Get());
	if (!SlotModule)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGatherMethod_001::GetGatheringLocation() - Failed to get SlotModule"));
		OutLocation = FVector::ZeroVector;
		return false;
	}
	
	// Try to take a slot
	bool bSlotFound = false;
	SlotModule->TakeSlot(GathererModule->Owner, bSlotFound, OutLocation);
	
	if (bSlotFound)
	{
		UE_LOG(LogTemp, Log, TEXT("UGatherMethod_001::GetGatheringLocation() - Slot found at location: %s"), *OutLocation.ToString());
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UGatherMethod_001::GetGatheringLocation() - No slot available"));
		OutLocation = FVector::ZeroVector;
		return false;
	}
}
