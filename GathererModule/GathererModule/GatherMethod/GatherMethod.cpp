#include "GatherMethod.h"
#include "GameFramework/Controller.h"
#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "GathererModule/GathererModule.h"
#include "RTS_Actor.h"
#include "GatherableModule/GatherableModule.h"
#include "SlotModule/SlotModule.h"
#include "Utilis/Libraries/RTSModuleFunctionLibrary.h"

void UGatherMethod::InitializeGatherMethod(UGathererModule* Gatherer)
{
	GathererModule = Gatherer;
	SetResourceTypePriority(EResourceType::Wood);
}

void UGatherMethod::Gather(ARTS_Actor* TargetResource)
{
	if (!TargetResource)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGatherMethod::Gather() - TargetResource is null!"));
		FindNewResource();
		return;
	}

	// Ensure gatherable module is available for this target
	if (!GatherableModule || CurrentGatheringTarget.Get() != TargetResource)
	{
		// Clear any active gathering timer when switching to a new resource
		if (GathererModule && GathererModule->GetWorld())
		{
			GathererModule->GetWorld()->GetTimerManager().ClearTimer(GatheringTimer);
		}
		
		GatherableModule = URTSModuleFunctionLibrary::GetGatherableModule(TargetResource);
		CurrentGatheringTarget = TargetResource;
	}

	if (!GatherableModule)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGatherMethod::Gather() - GatherableModule is null!"));
		return;
	}

	SetResourceTypePriority(GatherableModule->ResourceType);
}


void UGatherMethod::StartGathering()
{
	// Base implementation - empty
}

void UGatherMethod::TickGathering()
{
	// Base implementation - empty
}

void UGatherMethod::CompleteGathering()
{
	// Base implementation - empty
}

void UGatherMethod::StopGather()
{
	// Clear any active timers
	if (GathererModule && GathererModule->GetWorld())
	{
		GathererModule->GetWorld()->GetTimerManager().ClearTimer(GatheringTimer);
	}
	
	// Reset gathering state
	CurrentGatheringTime = 0.f;
	CurrentGatheringTarget = nullptr;
	GatherableModule = nullptr;
	CurrentGatheringTarget = nullptr;
	
	// Reset progress immediately when gathering completes
	GathererModule->OnGatheringProgress.Broadcast(0.0f, 0.0f);
}

bool UGatherMethod::GetGatheringLocation(FVector& OutLocation)
{	

	return false;
}

void UGatherMethod::SetResourceTypePriority(EResourceType ResourceType)
{
	ResourceTypePriority = ResourceType;
}

void UGatherMethod::FindNewResource()
{
	// NOTE: THIS WILL REQUIRE DELAY OR SET TIMER BY EVENT TO NOT GET INTO INFINITY LOOP
	//  (basically it's a retry every 0.1s or something like that)
	// TODO: implementing finding new resource and t hen call to gather method
	UE_LOG(LogTemp, Warning, TEXT("UGatherMethod::FindNewResource() - Finding new resource"));

	//ARTS_Actor* FoundResourceTarget = nullptr;
	//GathererModule->ExecuteGathererModule(FoundResourceTarget);
}