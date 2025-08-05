// Copyright AmberleafCotton 2025. All Rights Reserved.
#pragma once

#include "RTS_Module.h"
#include "ResourceType.h"
#include "ResourceSize.h"
#include "GatherableModule.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnResourceHarvested, int32, CurrentResourceAmount, int32, MaxResourceAmount, int32, ValueAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResourceDepleted);

/**
 * A module representing a gatherable resource.
 * Handles basic resource tracking and exposes a harvesting event.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class FINALRTS_API UGatherableModule : public URTS_Module
{
	GENERATED_BODY()

public:
	UGatherableModule();

	virtual void InitializeModule_Implementation(ARTS_Actor* InOwner) override;
	
	/** Resource type (e.g., Wood, Stone) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gatherable Module")
	EResourceType ResourceType = EResourceType::Wood;

	/** Resource size (e.g., Small, Normal, Large) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gatherable Module")
	EResourceSize ResourceSize = EResourceSize::Normal;
	
	/** Total amount of resource available at start */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gatherable Module")
	int32 ResourceAmount = 100;

	/** Current remaining resource amount */
	UPROPERTY()
	int32 CurrentResourceAmount = 0;

	/** Resource stack size (e.g., 1 stack = 5 wood) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gatherable Module")
	int32 ResourceStack = 1;

	/** Time needed to gather one stack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gatherable Module")
	float GatheringTime = 5.f;
	
	UFUNCTION(BlueprintPure, Category = "Gatherable Module")
	int32 GetCurrentResourceAmount() const;
	
	UFUNCTION(BlueprintPure, Category = "Gatherable Module")
	int32 GetResourceStackAmount() const;
	
	UFUNCTION(BlueprintCallable, Category = "Gatherable Module")
	void HarvestResource(int32 Amount, bool& OutHarvested, int32& OutStackAmount, EResourceType& OutResourceType);
	
	/** Called when a resource is gathered */
	UPROPERTY(BlueprintAssignable, Category = "Gatherable Module")
	FOnResourceHarvested OnResourceHarvested;
	
	/** Called when a resource is depleted */
	UPROPERTY(BlueprintAssignable, Category = "Gatherable Module")
	FOnResourceDepleted OnResourceDepleted;
};
