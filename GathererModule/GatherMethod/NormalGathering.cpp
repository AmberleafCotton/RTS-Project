#include "NormalGathering.h"

void UNormalGathering::StartGathering()
{
	CurrentGatheringTime = 0.f;
	
	RequiredGatheringTime = GatherableModule->GatheringTime;

	GathererModule->GetWorld()->GetTimerManager().SetTimer(GatheringTimer, this, &UNormalGathering::TickGathering, 0.2f, true);
}

void UNormalGathering::TickGathering()
{
	CurrentGatheringTime += 0.2f;

	if (!GathererModule || !GatherableModule) return;

	GathererModule->OnGatheringProgress.Broadcast(CurrentGatheringTime, RequiredGatheringTime);

	if (CurrentGatheringTime >= RequiredGatheringTime)
	{
		CompleteGathering();
	}
}
