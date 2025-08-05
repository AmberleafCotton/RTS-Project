#include "RTS_Actor.h"
#include "RTS_DataAsset.h"
#include "RTS_Module.h"

ARTS_Actor::ARTS_Actor()
{
	
}

void ARTS_Actor::Initialize_Implementation()
{
	// Process each module category in the data asset
	for (const TPair<FGameplayTag, TObjectPtr<URTS_Module>>& Pair : ActorDataAsset->Modules)
	{
		const FGameplayTag& Tag = Pair.Key;
		const TObjectPtr<URTS_Module>& Module = Pair.Value;

		// Skip invalid modules
		if (!Module)
		{
			continue;
		}

		// Create a copy of the module for this actor
		URTS_Module* DuplicatedModule = DuplicateObject<URTS_Module>(Module, this);
		if (!DuplicatedModule)
		{
			continue;
		}

		// Initialize the module with this actor as owner
		DuplicatedModule->InitializeModule(this);

		// Add the module to the appropriate category in this actor
		Modules.Add(Tag, DuplicatedModule);
	}
}

FGameplayTag ARTS_Actor::GetGameplayTag() const
{
	return ActorDataAsset->GameplayTag;
}

FGameplayTagContainer ARTS_Actor::GetRTS_Tags() const
{
	return ActorDataAsset->RTS_Tags;
}
