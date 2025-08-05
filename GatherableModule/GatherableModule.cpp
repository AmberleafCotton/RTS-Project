// Copyright AmberleafCotton 2025. All Rights Reserved.=
#include "GatherableModule.h"
#include "RTS_Actor.h"

UGatherableModule::UGatherableModule()
{
	
}

void UGatherableModule::InitializeModule_Implementation(ARTS_Actor* InOwner)
{
	Super::InitializeModule_Implementation(InOwner);

	switch (ResourceSize)
	{
	case EResourceSize::Normal:
		CurrentResourceAmount = ResourceAmount;
		break;
	case EResourceSize::Plentiful:
		// Placeholder
		break;
	case EResourceSize::Mega:
		// Placeholder
		break;
	default:
		CurrentResourceAmount = ResourceAmount;
		break;
	}
}

void UGatherableModule::HarvestResource(int32 Amount, bool& OutHarvested, int32& OutStackAmount, EResourceType& OutResourceType)
{
	// Default values before processing
	OutStackAmount = ResourceStack;
	OutResourceType = ResourceType;
	OutHarvested = false;

	if (CurrentResourceAmount <= 0)
	{
		// No resources to harvest
		OutHarvested = false;
		return;
	}

	// Perform the harvesting logic
	CurrentResourceAmount -= Amount;

	if (CurrentResourceAmount <= 0)
	{
		OnResourceDepleted.Broadcast();

		// Destroy the owner actor when resource is depleted
		if (Owner)
		{
			Owner->Destroy();
		}
	}

	OnResourceHarvested.Broadcast(CurrentResourceAmount, ResourceAmount, Amount);

	// Successfully harvested
	OutHarvested = true;
}


int32 UGatherableModule::GetCurrentResourceAmount() const
{
	return CurrentResourceAmount;
}

int32 UGatherableModule::GetResourceStackAmount() const
{
	return ResourceStack;
}

