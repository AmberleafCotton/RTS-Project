#include "NormalDeposit.h"

void UNormalDeposit::Deposit_Implementation(ARTS_Actor* Target)
{
	// Move to deposit target location
	if (Target)
	{
		MoveToLocation(Target->GetActorLocation());
	}
}

void UNormalDeposit::CompleteDepositing()
{
	// Actually deposit the stored resource here
	// Call GathererModule->OnDeposited or similar
}
