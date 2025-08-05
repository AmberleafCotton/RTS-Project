// Copyright AmberleafCotton 2025. All Rights Reserved.
#include "RTS_Module.h"
#include "RTS_Actor.h"

URTS_Module::URTS_Module()
{
	
}

void URTS_Module::InitializeModule_Implementation(ARTS_Actor* InOwner)
{
	Owner = InOwner;
}

UWorld* URTS_Module::GetWorld() const
{
	return Owner ? Owner->GetWorld() : nullptr;
}