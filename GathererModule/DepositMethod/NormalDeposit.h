#pragma once

#include "DepositMethod.h"
#include "NormalDeposit.generated.h"

UCLASS()
class DRAKTHYSPROJECT_API UNormalDeposit : public UDepositMethod
{
	GENERATED_BODY()

public:
	virtual void Deposit() override;
	virtual void CompleteDepositing() override;
	virtual void StopDeposit() override;
	
	virtual FVector GetDepositLocation() override;
};
