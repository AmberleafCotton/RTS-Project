#pragma once

#include "DepositMethod.h"
#include "TimerManager.h"
#include "InstantDeposit.generated.h"

UCLASS()
class DRAKTHYSPROJECT_API UInstantDeposit : public UDepositMethod
{
	GENERATED_BODY()

public:
	virtual void Deposit() override;
	virtual void CompleteDepositing() override;
	virtual void StopDeposit() override;
};
