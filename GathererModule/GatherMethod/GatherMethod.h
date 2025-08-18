// GatherMethod.h
#pragma once

#include "AIController.h"
#include "RTS_Actor.h"
#include "GatherableModule/GatherableModule.h"
#include "GathererModule/GathererModule.h"
#include "Navigation/PathFollowingComponent.h"
#include "SlotModule/SlotModule.h"
#include "GatherMethod.generated.h"

UCLASS(Abstract, Blueprintable, EditInlineNew)
class DRAKTHYSPROJECT_API UGatherMethod : public UObject
{
	GENERATED_BODY()

public:
	void InitializeGatherMethod(UGathererModule* Gatherer);
	
	UPROPERTY()
	UGathererModule* GathererModule;
	
	virtual void Gather(ARTS_Actor* ResourceTarget);
	virtual void StopGather();
	
	FTimerHandle GatheringTimer;
	
	UPROPERTY()
	TObjectPtr<UGatherableModule> GatherableModule;
	
	UPROPERTY()
	TWeakObjectPtr<ARTS_Actor> CurrentGatheringTarget;

	virtual bool GetGatheringLocation(FVector& OutLocation);
	
	float CurrentGatheringTime = 0.f;
	float RequiredGatheringTime = 0.f;

	void virtual StartGathering();
	void virtual TickGathering();
	void virtual CompleteGathering();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gather Method")
	bool bDrawDebugPath;

	EResourceType ResourceTypePriority;

	void virtual FindNewResource();
	void virtual SetResourceTypePriority(EResourceType ResourceType);
	// add FindNewResource
};
