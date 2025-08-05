#pragma once

#include "DepositMethod.h"
#include "NormalDeposit.generated.h"

UCLASS()
class FINALRTS_API UNormalDeposit : public UDepositMethod
{
	GENERATED_BODY()

public:
	virtual void Deposit_Implementation(ARTS_Actor* Target) override;
	virtual void CompleteDepositing() override;
};
