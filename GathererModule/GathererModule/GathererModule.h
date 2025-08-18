// Copyright 2025 AmberleafCotton. All rights reserved.
#pragma once

#include "ResourceTypes.h"
#include "RTS_Actor.h"
#include "RTS_Module.h"
#include "Navigation/PathFollowingComponent.h"
#include "GathererModule.generated.h"

class UDepositMethod;
class UGatherMethod;
class AAIController;
class UPathFollowingComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceGathered, ARTS_Actor*, ResourceTarget, int32, ResourceAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGatheringProgress, float, CurrentGatheringTime, float, RequiredGatheringTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceDeposited, EResourceType, ResourceType, int32, DepositedAmount);

UENUM(BlueprintType)
enum class EGathererState : uint8
{
	Idle,
	Gathering,
	Depositing
};

/**
 * A module that handles gathering logic for units.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class DRAKTHYSPROJECT_API UGathererModule : public URTS_Module
{
	GENERATED_BODY()

public:
	UGathererModule();
	
	virtual void InitializeModule_Implementation(ARTS_Actor* InOwner) override;

	UPROPERTY()
	TWeakObjectPtr<ARTS_Actor> TargetResource;

	UFUNCTION(BlueprintCallable, Category = "Gatherer Module")
	void ExecuteGathererModule(ARTS_Actor* InTargetResource);

	UFUNCTION(BlueprintCallable, Category = "Gatherer Module")
	void StopGathererModule();
	
	void ResourceGathered(int32 ResourceAmount, EResourceType ResourceType);
	
	UFUNCTION(BlueprintCallable, Category = "Gatherer Module")
	void ResourceDeposited(int32 DepositedAmount, EResourceType ResourceType);

	// Task 21: Module transition API (methods drive next steps)
	void RequestDeposit();
	void RequestContinueGather();

	// Movement functions for Task 12
	UFUNCTION(BlueprintCallable, Category = "Gatherer Module")
	void MoveToLocation(FVector Location);
	
	UFUNCTION(BlueprintCallable, Category = "Gatherer Module")
	void StopMovement();

	UPROPERTY(BlueprintAssignable, Category = "Gatherer Module")
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
	
	UPROPERTY(BlueprintReadOnly, Category = "Gatherer Module")
	EGathererState CurrentState = EGathererState::Idle;
	
	/** Called when a resource is gathered */
	UPROPERTY(BlueprintAssignable, Category = "Gatherer Module")
	FOnResourceGathered OnResourceGathered;
	
	/** Called when a resource is deposited */
	UPROPERTY(BlueprintAssignable, Category = "Gatherer Module")
	FOnResourceDeposited OnResourceDeposited;

private:
	// Cached movement references
	UPROPERTY()
	TObjectPtr<AAIController> CachedAIController = nullptr;
	
	UPROPERTY()
	TObjectPtr<UPathFollowingComponent> CachedPathComp = nullptr;
	
	// Movement event handling
	void OnMovementCompleted(FAIRequestID /*RequestID*/, const FPathFollowingResult& Result);
	void BindMovementEvents();
	void UnbindMovementEvents();
	
};
