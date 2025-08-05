// Copyright 2025 AmberleafCotton. All right reserved.
#include "SelectableComponent.h"

USelectableComponent::USelectableComponent()
{
    CurrentSelectionState = ESelectionState::Unselected;
    OwnershipType = EOwnershipType::Neutral;
}

void USelectableComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USelectableComponent::BroadcastSelectionState(ESelectionState NewState)
{
    OnSelectionStateUpdate.Broadcast(NewState);
    
    // Add logging directly here if needed
    UE_LOG(LogTemp, Log, TEXT("%s selection state updated to %d"), 
        *GetOwner()->GetName(), 
        static_cast<int32>(NewState));
}