// GatherMethod.h
#pragma once

#include "AIController.h"
#include "RTS_Actor.h"
#include "GatherableModule/GatherableModule.h"
#include "GathererModule/GathererModule.h"
#include "Navigation/PathFollowingComponent.h"
#include "GatherMethod.generated.h"

UCLASS(Abstract, Blueprintable, EditInlineNew)
class FINALRTS_API UGatherMethod : public UObject
{
	GENERATED_BODY()

public:
	void InitializeGatherMethod(UGathererModule* Gatherer);
	
	UPROPERTY()
	UGathererModule* GathererModule;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gather Method")
	void Gather(ARTS_Actor* ResourceTarget);
	virtual void Gather_Implementation(ARTS_Actor* ResourceTarget);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gather Method")
	void StopGather();
	virtual void StopGather_Implementation();
	
	FTimerHandle GatheringTimer;
	
	UPROPERTY()
	TObjectPtr<UGatherableModule> GatherableModule;
	
	float CurrentGatheringTime = 0.f;
	float RequiredGatheringTime = 0.f;

	void virtual OnMoveCompleted_Event(FAIRequestID RequestID, const FPathFollowingResult& Result);
	void virtual StartGathering();
	void virtual TickGathering();
	void CompleteGathering();
	
	void MoveToLocation(const FVector& TargetLocation);
	FDelegateHandle FOnMoveCompleted;

	UPROPERTY()
	TObjectPtr<AAIController> CachedAIController = nullptr;
	
	UPROPERTY()
	TObjectPtr<UPathFollowingComponent> CachedPathComp = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDrawDebugPath;
};
