// Copyright AmberleafCotton 2025. All Rights Reserved.
#pragma once

#include "RTS_Module.h"
#include "UnitDataAsset.h"
#include "RecruitmentModule.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProductionProgressUpdated, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProductionQueueUpdated, const TArray<UUnitDataAsset*>&, ProductionQueue);

/**
 * Base class for recruitment functionality in RTS game.
 * Abstract class that handles unit production queue and spawning.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class FINALRTS_API URecruitmentModule : public URTS_Module
{
	GENERATED_BODY()

public:
	URecruitmentModule();
	
	virtual void InitializeModule_Implementation(ARTS_Actor* InOwner) override;

	/** Adds a unit to the production queue */
	UFUNCTION(BlueprintCallable, Category = "Recruitment Module")
	void AddUnitToProduction(UUnitDataAsset* UnitDataAsset);

	/** Returns the available units for production */
	UFUNCTION(BlueprintPure, Category = "Recruitment Module")
	TArray<UUnitDataAsset*> GetUnitsForProduction() const { return UnitsForProduction; }

	/** Returns the current production queue */
	UFUNCTION(BlueprintPure, Category = "Recruitment Module")
	TArray<UUnitDataAsset*> GetProductionQueue() const { return UnitProductionQueue; }

protected:
	/** Called when production queue processing should begin */
	virtual void EnableProduction();
	
	/** Processes the current production queue */
	virtual void ProcessProductionQueue();
	
	/** Spawns the currently produced unit */
	UFUNCTION(BlueprintNativeEvent, Category = "Recruitment Module")
	void SpawnUnit();

	/** Units available for production in this module */
	UPROPERTY(EditDefaultsOnly, Category = "Recruitment Module")
	TArray<TObjectPtr<UUnitDataAsset>> UnitsForProduction;

	/** Current queue of units to be produced */
	UPROPERTY(BlueprintReadOnly, Category = "Recruitment Module")
	TArray<TObjectPtr<UUnitDataAsset>> UnitProductionQueue;

	/** Currently being produced unit */
	UPROPERTY(BlueprintReadOnly, Category = "Recruitment Module")
	TObjectPtr<UUnitDataAsset> UnitBeingProduced = nullptr;

	/** Time spent on current production */
	UPROPERTY(BlueprintReadOnly, Category = "Recruitment Module")
	float ProductionTimeSpent = 0.0f;

	/** Time needed for current production */
	UPROPERTY(BlueprintReadOnly, Category = "Recruitment Module")
	float ProductionTimeNeeded = 0.0f;

	/** Current production progress (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Recruitment Module")
	float ProductionProgress = 0.0f;

	/** How often to update production progress */
	UPROPERTY(EditDefaultsOnly, Category = "Recruitment Module")
	float ProductionTimerGranularity = 0.2f;

	/** Whether a unit is currently being produced */
	UPROPERTY(BlueprintReadOnly, Category = "Recruitment Module")
	bool bIsProducingUnit = false;

	/** Timer handle for production updates */
	FTimerHandle ProductionTimerHandle;

	/** Delegate for production progress updates */
	UPROPERTY(BlueprintAssignable, Category = "Recruitment Module")
	FOnProductionProgressUpdated OnProductionProgressUpdated;

	/** Delegate for queue updates */
	UPROPERTY(BlueprintAssignable, Category = "Recruitment Module")
	FOnProductionQueueUpdated OnProductionQueueUpdated;
};
