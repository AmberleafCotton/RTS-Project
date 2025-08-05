// Copyright 2025 AmberleafCotton. All rights reserved.
#pragma once

#include "TeamSettings.h"
#include "RTS_Component.h"
#include "TeamComponent.generated.h"

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FINALRTS_API UTeamComponent : public URTS_Component
{
	GENERATED_BODY()

public:
	UTeamComponent();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Team Component")
	void InitializeTeam(const FTeamSettings InTeamSettings, APlayerState* InPlayerState);

	UFUNCTION(BlueprintPure, Category = "Team Component")
	void GetTeamInfo(FTeamSettings& OutTeamSettings, APlayerState*& OutPlayerState) const
	{
		OutTeamSettings = TeamSettings;
		OutPlayerState = OwningPlayerState;
	}

	UFUNCTION(BlueprintPure, Category = "Team Component")
	APlayerState* GetPlayerOwner() const
	{
		return OwningPlayerState;
	}

	UFUNCTION(BlueprintPure, Category = "Team Component")
	FTeamSettings GetTeamSettings() const
	{
		return TeamSettings;
	}

	UFUNCTION(BlueprintCallable, Category = "Team Component")
	void SetTeamSettings(const FTeamSettings& NewTeamSettings)
	{
		TeamSettings = NewTeamSettings;
	}

	UFUNCTION(BlueprintCallable, Category = "Team Component")
	void SetPlayerOwner(APlayerState* NewPlayerState)
	{
		OwningPlayerState = NewPlayerState;
	}

	UFUNCTION(BlueprintPure, Category = "Team Component")
	int GetTeamIndex() const
	{
		return TeamSettings.TeamIndex;
	}

	UFUNCTION(BlueprintPure, Category = "Team Component")
	bool IsOwned(const UTeamComponent* OtherTeamComponent) const;

	UFUNCTION(BlueprintPure, Category = "Team Component")
	bool IsEnemy(const UTeamComponent* OtherTeamComponent) const;

	UFUNCTION(BlueprintPure, Category = "Team Component")
	bool IsAlly(const UTeamComponent* OtherTeamComponent) const;

	UFUNCTION(BlueprintPure, Category = "Team Component")
	bool IsNeutral() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team Component")
	FTeamSettings TeamSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team Component")
	APlayerState* OwningPlayerState;
};
