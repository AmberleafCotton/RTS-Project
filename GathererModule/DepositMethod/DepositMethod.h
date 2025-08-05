// DepositMethod.h
#pragma once

#include "AIController.h"
#include "RTS_Actor.h"
#include "GathererModule/GathererModule.h"
#include "Navigation/PathFollowingComponent.h"
#include "DepositMethod.generated.h"

UCLASS(Abstract, Blueprintable, EditInlineNew)
class FINALRTS_API UDepositMethod : public UObject
{
	GENERATED_BODY()

public:
	void InitializeDepositMethod(UGathererModule* Gatherer);

	UPROPERTY()
	UGathererModule* GathererModule;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Deposit Method")
	void Deposit(ARTS_Actor* Target);
	virtual void Deposit_Implementation(ARTS_Actor* Target);
	
	void GetDepositBuilding();

	void virtual CompleteDepositing();
	
	void virtual OnMoveCompleted_Event(FAIRequestID RequestID, const FPathFollowingResult& Result);

	void MoveToLocation(const FVector& TargetLocation);
	
	FDelegateHandle FOnMoveCompleted;
	
	UPROPERTY()
	TObjectPtr<AAIController> CachedAIController = nullptr;
	
	UPROPERTY()
	TObjectPtr<UPathFollowingComponent> CachedPathComp = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDrawDebugPath;
};
