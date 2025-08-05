#pragma once

#include "DepositMethod.h"
#include "InstantDeposit.generated.h"

UCLASS()
class FINALRTS_API UInstantDeposit : public UDepositMethod
{
	GENERATED_BODY()

public:
	virtual void Deposit_Implementation(ARTS_Actor* Target) override;
};
