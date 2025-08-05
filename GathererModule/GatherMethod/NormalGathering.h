// Copyright AmberleafCotton 2025. All Rights Reserved.
#pragma once

#include "GatherMethod.h"
#include "NormalGathering.generated.h"

UCLASS(Blueprintable, EditInlineNew)
class FINALRTS_API UNormalGathering : public UGatherMethod
{
	GENERATED_BODY()

public:
	
	//void OnMoveCompleted_Event(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
	virtual void StartGathering() override;
	virtual void TickGathering() override;
};
