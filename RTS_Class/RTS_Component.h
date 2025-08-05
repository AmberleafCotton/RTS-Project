#pragma once

#include "RTS_Actor.h"
#include "Components/ActorComponent.h"
#include "RTS_Component.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FINALRTS_API URTS_Component : public UActorComponent
{
	GENERATED_BODY()

public:
	URTS_Component();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RTS Component")
	void InitializeRTSComponent(ARTS_Actor* InOwner);
	virtual void InitializeRTSComponent_Implementation(ARTS_Actor* InOwner);

	UPROPERTY(BlueprintReadWrite, Category = "RTS Component")
	ARTS_Actor* Owner = nullptr;

	/** Returns the owner of this widget */
	UFUNCTION(BlueprintPure, Category = "RTS Component")
	ARTS_Actor* GetComponentOwner() const;
};
