// Copyright AmberleafCotton 2025. All Rights Reserved.
#pragma once

#include "GatherMethod.h"
#include "SlotModule/SlotModule.h"
#include "GatherMethod_001.generated.h"

UCLASS(Blueprintable, EditInlineNew)
class DRAKTHYSPROJECT_API UGatherMethod_001 : public UGatherMethod
{
	GENERATED_BODY()

public:
// override to create your own method
	virtual void Gather(ARTS_Actor* ResourceTarget) override;
	virtual void StopGather() override;
	
	virtual bool GetGatheringLocation(FVector& OutLocation) override;

	virtual void StartGathering() override;
	virtual void TickGathering() override;
	virtual void CompleteGathering() override;
	

	UPROPERTY()
	TObjectPtr<USlotModule> SlotModule;

	// Method-local storage policy: stacks based
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gather Method")
	int32 StacksStorageAmount = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Gather Method")
	int32 CurrentGatheredStacks = 0;
};
