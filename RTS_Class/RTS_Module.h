// Copyright AmberleafCotton 2025. All Rights Reserved.
#pragma once

#include "UObject/Object.h"
#include "RTS_Module.generated.h"

class ARTS_Actor;

/**
 * Base class for all RTS Modules.
 * Provides common behavior and structure for modular systems.
 */
UCLASS(Blueprintable, BlueprintType)
class FINALRTS_API URTS_Module : public UObject
{
	GENERATED_BODY()

public:
	URTS_Module();

	// Parent class (RTS Actor or base class)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RTS Module")
	void InitializeModule(ARTS_Actor* InOwner);
	virtual void InitializeModule_Implementation(ARTS_Actor* InOwner);
	
	UPROPERTY()
	ARTS_Actor* Owner = nullptr;

	UFUNCTION()
	UWorld* GetWorld() const;
	
	/** Returns the owner of this module */
	UFUNCTION(BlueprintPure, Category = "Recruitment Module")
	ARTS_Actor* GetModuleOwner() const { return Owner; }
};
