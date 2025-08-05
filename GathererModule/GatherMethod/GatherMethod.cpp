#include "GatherMethod.h"
#include "GameFramework/Controller.h"
#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "GathererModule/GathererModule.h"
#include "RTS_Actor.h"
#include "GatherableModule/GatherableModule.h"

void UGatherMethod::InitializeGatherMethod(UGathererModule* Gatherer)
{
	GathererModule = Gatherer;

	// Get Cached Path Comp from Pawn.
	if (APawn* OwnerPawn = Cast<APawn>(GathererModule->Owner))
	{
		
		CachedAIController = Cast<AAIController>(OwnerPawn->GetController());

		if (CachedAIController)
		{
			CachedPathComp = CachedAIController->GetPathFollowingComponent();
		}
	}

	// Bind to on arrival at resource position.
	if (!FOnMoveCompleted.IsValid())
	{
		FOnMoveCompleted = CachedPathComp->OnRequestFinished.AddUObject(this, &UGatherMethod::OnMoveCompleted_Event);
	}
}


void UGatherMethod::Gather_Implementation(ARTS_Actor* TargetResource)
{
	GatherableModule = Cast<UGatherableModule>(TargetResource->Modules[FGameplayTag::RequestGameplayTag("Module.Gatherable")]);
	
	RequiredGatheringTime = GatherableModule->GatheringTime;
	
	FVector TargetLocation = TargetResource->GetActorLocation();
	MoveToLocation(TargetLocation);
}

void UGatherMethod::StopGather_Implementation()
{

}

void UGatherMethod::MoveToLocation(const FVector& TargetLocation)
{
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(TargetLocation);
	MoveRequest.SetAcceptanceRadius(150.f);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetAllowPartialPath(false);

	FNavPathSharedPtr NavPath;
	CachedAIController->MoveTo(MoveRequest, &NavPath);

	if (bDrawDebugPath)
	{
		if (NavPath.IsValid() && NavPath->GetPathPoints().Num() > 1)
			for (int32 i = 0; i < NavPath->GetPathPoints().Num() - 1; ++i)
				DrawDebugLine(GetWorld(),
		NavPath->GetPathPoints()[i].Location,
		 NavPath->GetPathPoints()[i + 1].Location,
		  FColor::Green,false, 5.0f, 0, 4.0f);
	}
}

void UGatherMethod::OnMoveCompleted_Event(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (Result.Code == EPathFollowingResult::Success)
	{
		StartGathering();
	}
}

void UGatherMethod::StartGathering()
{
	
}

void UGatherMethod::TickGathering()
{
	
}

void UGatherMethod::CompleteGathering()
{
	if (!GathererModule || !GatherableModule) return;

	GathererModule->GetWorld()->GetTimerManager().ClearTimer(GatheringTimer);

	bool bHarvested = false;
	int32 OutAmount = 0;
	EResourceType OutType;

	GatherableModule->HarvestResource(1, bHarvested, OutAmount, OutType);

	if (bHarvested)
	{
		GathererModule->ResourceGathered(OutAmount, OutType);
	}
	else
	{
		// Find New Resource Placeholder
	}
}


