// DepositMethod.cpp
#include "DepositMethod.h"

void UDepositMethod::InitializeDepositMethod(UGathererModule* Gatherer)
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
		FOnMoveCompleted = CachedPathComp->OnRequestFinished.AddUObject(this, &UDepositMethod::OnMoveCompleted_Event);
	}
}


void UDepositMethod::Deposit_Implementation(ARTS_Actor* Target)
{
	// Default deposit logic (empty)
}

void UDepositMethod::MoveToLocation(const FVector& TargetLocation)
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

void UDepositMethod::GetDepositBuilding()
{
	
}

void UDepositMethod::OnMoveCompleted_Event(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (Result.Code == EPathFollowingResult::Success)
	{
		CompleteDepositing();
	}
}

void UDepositMethod::CompleteDepositing()
{
	
}