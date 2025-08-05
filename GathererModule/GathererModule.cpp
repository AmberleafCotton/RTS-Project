// Copyright AmberleafCotton 2025. All Rights Reserved.
#include "GathererModule.h"
#include "GameFramework/Actor.h"
#include "GatherMethod/GatherMethod.h"
#include "DepositMethod/DepositMethod.h"
#include "DrawDebugHelpers.h"

UGathererModule::UGathererModule()
{
	
}

void UGathererModule::InitializeModule_Implementation(ARTS_Actor* InOwner)
{
	Super::InitializeModule_Implementation(InOwner);
	GatherMethod->InitializeGatherMethod(this);
	//DepositMethod->InitializeDepositMethod();
}

void UGathererModule::ExecuteGathererModule(ARTS_Actor* InTargetResource)
{
	TargetResource = InTargetResource;
	if (CurrentResourceAmount >= MaxResourceStorage)
	{
		DepositMethod->Deposit(InTargetResource);
	}
	else
	{
		GatherMethod->Gather(InTargetResource);
	}
}

void UGathererModule::ResourceGathered(int32 ResourceAmount, EResourceType ResourceType)
{
	CurrentResourceAmount = ResourceAmount;
	CurrentResourceType = ResourceType;
	OnResourceGathered.Broadcast(TargetResource.Get(), ResourceAmount);
}
