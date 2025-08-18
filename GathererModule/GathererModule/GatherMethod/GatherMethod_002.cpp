#include "GatherMethod_002.h"
#include "TimerManager.h"
#include "Utilis/Libraries/RTSModuleFunctionLibrary.h"

void UGatherMethod_002::Gather(ARTS_Actor* TargetResource)
{
	Super::Gather(TargetResource);

	// Method 002 Policy: if carrying different resource type than target, deposit first
	if (GathererModule->CurrentResourceAmount > 0 && GathererModule->CurrentResourceType != GatherableModule->ResourceType)
	{
		GathererModule->RequestDeposit();
		return;
	}

	// Method 002 Policy: If we recently deposited, reset internal stored units
	if (GathererModule->CurrentResourceAmount == 0 && CurrentStoredUnits > 0)
	{
		CurrentStoredUnits = 0;
	}

	// Method 002 Policy: if storage full by units, deposit
	if (CurrentStoredUnits >= StoragePower)
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
			UE_LOG(LogTemp, Log, TEXT("UGatherMethod_002::Gather() - Already at gathering location; starting gathering. Loc=%s"), *GathererLocation.ToString());
			StartGathering();
		}
		else
		{
			// Otherwise move to the gathering location
			UE_LOG(LogTemp, Log, TEXT("UGatherMethod_002::Gather() - Moving to gathering location. From=%s To=%s Dist2D=%.2f"), *GathererLocation.ToString(), *GatheringLocation.ToString(), FVector::Dist2D(GathererLocation, GatheringLocation));
			GathererModule->MoveToLocation(GatheringLocation);
		}
	}
	else
	{
		// No valid gathering location found
		UE_LOG(LogTemp, Warning, TEXT("UGatherMethod_002::Gather() - No valid gathering location found"));
	}
}

void UGatherMethod_002::StartGathering()
{
	CurrentGatheringTime = 0.f;
	RequiredGatheringTime = GatherableModule->GatheringTime;
	GathererModule->GetWorld()->GetTimerManager().SetTimer(GatheringTimer, this, &UGatherMethod_002::TickGathering, 0.2f, true);
}

void UGatherMethod_002::TickGathering()
{
	CurrentGatheringTime += 0.2f;

	if (!GathererModule || !GatherableModule) return;

	GathererModule->OnGatheringProgress.Broadcast(CurrentGatheringTime, RequiredGatheringTime);

	if (CurrentGatheringTime >= RequiredGatheringTime)
	{
		CompleteGathering();
	}
}

void UGatherMethod_002::CompleteGathering()
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
	EResourceType OutType;
	int32 OutHarvestedAmount = 0;

	// Harvest a raw amount per cycle defined by HarvestPower
	GatherableModule->HarvestResource(HarvestPower, bHarvested, OutType, OutHarvestedAmount);

	if (bHarvested)
	{
		// Inform module of gather event
		GathererModule->ResourceGathered(OutHarvestedAmount, OutType);
		// Track method-local storage in units
		CurrentStoredUnits = FMath::Clamp(CurrentStoredUnits + OutHarvestedAmount, 0, StoragePower);

		// Re-enter via module to make the next decision
		if (CurrentGatheringTarget.IsValid())
		{
			GathererModule->ExecuteGathererModule(CurrentGatheringTarget.Get());
		}
	}
}

void UGatherMethod_002::StopGather()
{
	// Release the slot using the cached SlotModule
	if (SlotModule && GathererModule && GathererModule->Owner)
	{
		SlotModule->FreeUpSlot(GathererModule->Owner);
	}
	Super::StopGather();
}

bool UGatherMethod_002::GetGatheringLocation(FVector& OutLocation)
{
	// This method uses slot-based gathering (same as Method_001)
	if (!CurrentGatheringTarget.IsValid())
	{
		OutLocation = FVector::ZeroVector;
		return false;
	}
	
	// Get SlotModule for the target resource
	SlotModule = URTSModuleFunctionLibrary::GetSlotModule(CurrentGatheringTarget.Get());
	if (!SlotModule)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGatherMethod_002::GetGatheringLocation() - Failed to get SlotModule"));
		OutLocation = FVector::ZeroVector;
		return false;
	}
	
	// Try to take a slot
	bool bSlotFound = false;
	SlotModule->TakeSlot(GathererModule->Owner, bSlotFound, OutLocation);
	
	if (bSlotFound)
	{
		UE_LOG(LogTemp, Log, TEXT("UGatherMethod_002::GetGatheringLocation() - Slot found at location: %s"), *OutLocation.ToString());
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UGatherMethod_002::GetGatheringLocation() - No slot available"));
		OutLocation = FVector::ZeroVector;
		return false;
	}
}


