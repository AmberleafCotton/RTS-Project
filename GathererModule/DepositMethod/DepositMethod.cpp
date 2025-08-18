// DepositMethod.cpp
#include "DepositMethod.h"

void UDepositMethod::InitializeDepositMethod(UGathererModule* Gatherer)
{
	GathererModule = Gatherer;
}

void UDepositMethod::Deposit()
{
	// Intentionally left empty in base class
}

FVector UDepositMethod::GetDepositLocation()
{
	// Base implementation - return zero vector
	return FVector::ZeroVector;
	// This will use DepositModule later which will be different modules returning different locations (from building, from slot, etc.)
}

void UDepositMethod::CompleteDepositing()
{
	// Base implementation - empty
}

void UDepositMethod::StopDeposit()
{
	// Clear any active timers
	if (GathererModule && GathererModule->GetWorld())
	{
		GathererModule->GetWorld()->GetTimerManager().ClearTimer(DepositTimer);
	}
}