// Copyright 2025 AmberleafCotton. All rights reserved.
#pragma once

#include "ResourceType.h"
#include "RTS_Actor.h"
#include "RTS_Module.h"
#include "GathererModule.generated.h"

class UDepositMethod;
class UGatherMethod;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceGathered, ARTS_Actor*, ResourceTarget, float, ResourceAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGatheringProgress, float, CurrentGatheringTime, float, RequiredGatheringTime);


/**
 * A module that handles gathering logic for units.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class FINALRTS_API UGathererModule : public URTS_Module
{
	GENERATED_BODY()

public:
	UGathererModule();
	
	virtual void InitializeModule_Implementation(ARTS_Actor* InOwner) override;

	UPROPERTY()
	TWeakObjectPtr<ARTS_Actor> TargetResource;
	
	UFUNCTION(BlueprintCallable, Category = "Gatherer Module")
	void ExecuteGathererModule(ARTS_Actor* InTargetResource);
	
	void ResourceGathered(int32 ResourceAmount, EResourceType ResourceType);

	UPROPERTY(BlueprintAssignable, Category = "Gatherer")
	FOnGatheringProgress OnGatheringProgress;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Gatherer Module")
	TObjectPtr<UGatherMethod> GatherMethod;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Gatherer Module")
	TObjectPtr<UDepositMethod> DepositMethod;
	
	UPROPERTY(BlueprintReadWrite, Category = "Gatherer Module")
	int32 CurrentResourceAmount = 0;
	
	UPROPERTY(BlueprintReadWrite, Category = "Gatherer Module")
	EResourceType CurrentResourceType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gatherer Module")
	int32 MaxResourceStorage = 1;
	
	/** Called when a resource is gathered */
	UPROPERTY(BlueprintAssignable, Category = "Gatherable Module")
	FOnResourceGathered OnResourceGathered;
	
};
