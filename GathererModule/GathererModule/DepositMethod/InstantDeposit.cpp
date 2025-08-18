#include "InstantDeposit.h"
#include "Utilis/Libraries/RTSModuleFunctionLibrary.h"

void UInstantDeposit::Deposit()
{
	// Set a timer to perform the actual deposit after a short delay
	if (GathererModule->GetWorld())
	{ GathererModule->GetWorld()->GetTimerManager().SetTimer(DepositTimer, this, &UInstantDeposit::CompleteDepositing, 0.5f, false); }
}

void UInstantDeposit::CompleteDepositing()
{
	UPlayerResourcesModule* PlayerResources = URTSModuleFunctionLibrary::GetPlayerResources(GathererModule->Owner);
	if (!PlayerResources)
	{
		return;
	}

	if (GathererModule->CurrentResourceAmount > 0)
	{
		PlayerResources->AddResource(GathererModule->CurrentResourceType, GathererModule->CurrentResourceAmount);
		GathererModule->ResourceDeposited(GathererModule->CurrentResourceAmount, GathererModule->CurrentResourceType);
	}

	// Continue the loop
	GathererModule->RequestContinueGather();
}

void UInstantDeposit::StopDeposit()
{
	// Clear the deposit timer if it's active
	if (GathererModule && GathererModule->GetWorld())
	{
		GathererModule->GetWorld()->GetTimerManager().ClearTimer(DepositTimer);
	}
	
	// Call base implementation
	Super::StopDeposit();
}