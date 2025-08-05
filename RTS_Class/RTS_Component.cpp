// Copyright AmberleafCotton 2025. All Rights Reserved.
#include "RTS_Component.h"

URTS_Component::URTS_Component()
{
	
}

void URTS_Component::InitializeRTSComponent_Implementation(ARTS_Actor* InOwner)
{
	Owner = InOwner;
}

ARTS_Actor* URTS_Component::GetComponentOwner() const
{
	return Owner;
}

