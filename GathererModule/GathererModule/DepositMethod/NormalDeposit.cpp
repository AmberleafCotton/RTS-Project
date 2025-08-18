#include "NormalDeposit.h"
#include "Utilis/Libraries/RTSModuleFunctionLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UNormalDeposit::Deposit()
{
	// Get deposit location and move there
	FVector DepositLocation = GetDepositLocation();
	
		// If already close enough, complete immediately; otherwise move
		const float AcceptanceRadius = 25.0f; // match GathererModule MoveToLocation acceptance
		const FVector OwnerLocation = (GathererModule && GathererModule->Owner) ? GathererModule->Owner->GetActorLocation() : FVector::ZeroVector;
		if (FVector::DistSquared2D(OwnerLocation, DepositLocation) <= FMath::Square(AcceptanceRadius))
		{
			CompleteDepositing();
		}
		else
		{
			// Use GathererModule's movement
			GathererModule->MoveToLocation(DepositLocation);
		}
}

FVector UNormalDeposit::GetDepositLocation()
{
	return FVector::ZeroVector;
}

void UNormalDeposit::CompleteDepositing()
{
	if (!GathererModule)
	{
		return;
	}
	
	if (!GathererModule->Owner)
	{
		return;
	}

	// Get the player resources module
	UPlayerResourcesModule* PlayerResources = URTSModuleFunctionLibrary::GetPlayerResources(GathererModule->Owner);
	if (!PlayerResources)
	{
		return;
	}

	// Add the gathered resources to player's resources
	if (GathererModule->CurrentResourceAmount > 0)
	{
		PlayerResources->AddResource(GathererModule->CurrentResourceType, GathererModule->CurrentResourceAmount);
		GathererModule->ResourceDeposited(GathererModule->CurrentResourceAmount, GathererModule->CurrentResourceType);

		// Continue the cycle: request gather again via module single-entry flow
		GathererModule->RequestContinueGather();
	}
	else
	{
	}
}

void UNormalDeposit::StopDeposit()
{
	// Base class handles timer clearing
	Super::StopDeposit();
}