// DepositMethod.h
#pragma once

#include "AIController.h"
#include "RTS_Actor.h"
#include "GathererModule/GathererModule.h"
#include "Navigation/PathFollowingComponent.h"
#include "TimerManager.h"
#include "DepositMethod.generated.h"

UCLASS(Abstract, Blueprintable, EditInlineNew)
class DRAKTHYSPROJECT_API UDepositMethod : public UObject
{
	GENERATED_BODY()

public:
	void InitializeDepositMethod(UGathererModule* Gatherer);

	UPROPERTY()
	UGathererModule* GathererModule;
	
	virtual void Deposit();
	virtual void StopDeposit();
	virtual void CompleteDepositing();
	
	virtual FVector GetDepositLocation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDrawDebugPath;

	FTimerHandle DepositTimer;
};
