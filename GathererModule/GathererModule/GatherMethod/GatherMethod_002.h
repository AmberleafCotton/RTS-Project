// Copyright AmberleafCotton 2025. All Rights Reserved.
#pragma once

#include "GatherMethod.h"
#include "SlotModule/SlotModule.h"
#include "GatherMethod_002.generated.h"

UCLASS(Blueprintable, EditInlineNew)
class DRAKTHYSPROJECT_API UGatherMethod_002 : public UGatherMethod
{
	GENERATED_BODY()

public:
	virtual void Gather(ARTS_Actor* ResourceTarget) override;
	virtual void StopGather() override;

	virtual void StartGathering() override;
	virtual void TickGathering() override;
	virtual void CompleteGathering() override;
	
	// Method-specific gathering location logic
	virtual bool GetGatheringLocation(FVector& OutLocation) override;

	UPROPERTY()
	TObjectPtr<USlotModule> SlotModule;

	// Configurable power values
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gather Method")
	int32 HarvestPower = 1; // units per cycle

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gather Method")
	int32 StoragePower = 5; // max units before deposit

	// Internal storage tracking
	UPROPERTY(BlueprintReadWrite, Category = "Gather Method")
	int32 CurrentStoredUnits = 0;
};


