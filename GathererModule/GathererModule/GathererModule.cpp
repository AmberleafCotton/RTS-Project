// Copyright AmberleafCotton 2025. All Rights Reserved.
#include "GathererModule.h"
#include "GameFramework/Actor.h"
#include "GatherMethod/GatherMethod.h"
#include "DepositMethod/DepositMethod.h"
#include "GatherableModule/GatherableModule.h"
#include "DrawDebugHelpers.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Pawn.h"

UGathererModule::UGathererModule()
{
	// Constructor
}

void UGathererModule::InitializeModule_Implementation(ARTS_Actor* InOwner)
{
	Super::InitializeModule_Implementation(InOwner);

	// Cache AI controller and path following component
	if (APawn* OwnerPawn = Cast<APawn>(Owner))
	{
		CachedAIController = Cast<AAIController>(OwnerPawn->GetController());
		if (CachedAIController)
		{
			CachedPathComp = CachedAIController->GetPathFollowingComponent();
		}
	}

	if (GatherMethod)
	{
		GatherMethod->InitializeGatherMethod(this);
	}
	if (DepositMethod)
	{
		DepositMethod->InitializeDepositMethod(this);
	}
}

void UGathererModule::ExecuteGathererModule(ARTS_Actor* InTargetResource)
{
	TargetResource = InTargetResource;

	// Neutral coordinator: always enter Gathering; methods decide policy and transitions
	CurrentState = EGathererState::Gathering;
    if (GatherMethod)
	{
        UE_LOG(LogTemp, Verbose, TEXT("UGathererModule::ExecuteGather - forwarding to GatherMethod. Target=%s"), InTargetResource ? *InTargetResource->GetName() : TEXT("null"));
		GatherMethod->Gather(InTargetResource);
	}
}

void UGathererModule::StopGathererModule()
{
	UnbindMovementEvents();
	CurrentState = EGathererState::Idle;
	TargetResource = nullptr;

	if (GatherMethod)
	{
		GatherMethod->StopGather();
	}

	if (DepositMethod)
	{
		DepositMethod->StopDeposit();
	}
}

void UGathererModule::MoveToLocation(FVector Location)
{
	if (CachedAIController)
	{
        UE_LOG(LogTemp, Log, TEXT("UGathererModule::MoveToLocation() - Starting movement to: %s"), *Location.ToString());
        
        // Stop any current movement first to ensure clean state
        CachedAIController->StopMovement();
        
        // Unbind previous events
        UnbindMovementEvents();

        // Bind new events
        BindMovementEvents();

        // Execute movement
        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalLocation(Location);
        MoveRequest.SetAcceptanceRadius(1.0f); 
		
        CachedAIController->MoveTo(MoveRequest);
	}
}

void UGathererModule::StopMovement()
{
	if (CachedAIController)
	{
		CachedAIController->StopMovement();
		UnbindMovementEvents();
	}
}

void UGathererModule::BindMovementEvents()
{
	if (CachedPathComp)
	{
		UE_LOG(LogTemp, Log, TEXT("UGathererModule::BindMovementEvents() - Binding movement completion event"));
		CachedPathComp->OnRequestFinished.AddUObject(this, &UGathererModule::OnMovementCompleted);
	}
}

void UGathererModule::UnbindMovementEvents()
{
	if (CachedPathComp)
	{
		UE_LOG(LogTemp, Log, TEXT("UGathererModule::UnbindMovementEvents() - Unbinding movement completion event"));
		CachedPathComp->OnRequestFinished.RemoveAll(this);
	}
}

void UGathererModule::OnMovementCompleted(FAIRequestID /*RequestID*/, const FPathFollowingResult& Result)
{
    UE_LOG(LogTemp, Log, TEXT("UGathererModule::OnMovementCompleted() - Result: %s"), 
           Result.Code == EPathFollowingResult::Success ? TEXT("Success") : TEXT("Failed"));
    
    if (Result.Code == EPathFollowingResult::Success)
	{
		UnbindMovementEvents();
		UE_LOG(LogTemp, Log, TEXT("UGathererModule::OnMovementCompleted() - Re-entering ExecuteGathererModule"));
		ExecuteGathererModule(TargetResource.Get());
	}
    else
    {
        // Failure: ensure we do not leak bindings; optional retry/abort policy could be added later
        // Maybe add delay here to retry later
        UE_LOG(LogTemp, Warning, TEXT("UGathererModule::OnMovementCompleted() - Movement failed, unbinding events"));
        UnbindMovementEvents();
    }
}

void UGathererModule::ResourceGathered(int32 ResourceAmount, EResourceType ResourceType)
{
	// Event-only: update minimal state + broadcast
	CurrentResourceAmount += ResourceAmount; // accumulation fix
	CurrentResourceType = ResourceType;
	OnResourceGathered.Broadcast(TargetResource.Get(), ResourceAmount);
}

void UGathererModule::ResourceDeposited(int32 DepositedAmount, EResourceType ResourceType)
{
	// Event-only: update minimal state + broadcast
	CurrentResourceAmount = 0;
	OnResourceDeposited.Broadcast(ResourceType, DepositedAmount);
}

void UGathererModule::RequestDeposit()
{
	CurrentState = EGathererState::Depositing;
	if (DepositMethod)
	{
		DepositMethod->Deposit();
	}
}

void UGathererModule::RequestContinueGather()
{
	CurrentState = EGathererState::Gathering;
	if (GatherMethod)
	{
		GatherMethod->Gather(TargetResource.Get());
	}
}