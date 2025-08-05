// Copyright 2025 AmberleafCotton. All rights reserved.
#include "TeamComponent.h"

UTeamComponent::UTeamComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UTeamComponent::IsOwned(const UTeamComponent* OtherTeamComponent) const
{
	return OtherTeamComponent && OwningPlayerState == OtherTeamComponent->OwningPlayerState;
}

bool UTeamComponent::IsEnemy(const UTeamComponent* OtherTeamComponent) const
{
	return OtherTeamComponent &&
		TeamSettings.TeamIndex != 0 && TeamSettings.TeamIndex != OtherTeamComponent->TeamSettings.TeamIndex;
}

bool UTeamComponent::IsAlly(const UTeamComponent* OtherTeamComponent) const
{
	return OtherTeamComponent &&
		TeamSettings.TeamIndex == OtherTeamComponent->TeamSettings.TeamIndex;
}

bool UTeamComponent::IsNeutral() const
{
	return TeamSettings.TeamIndex == 0;
}
