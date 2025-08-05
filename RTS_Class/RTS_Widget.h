// Copyright AmberleafCotton 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RTS_Actor.h"
#include "Blueprint/UserWidget.h"
#include "RTS_Widget.generated.h"

/**
 * Base class for all RTS UI Widgets.
 */
UCLASS(Abstract)
class FINALRTS_API URTS_Widget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Initialize the widget with the owner actor */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RTS Widget")
	void InitializeWidget(ARTS_Actor* InOwner);
	virtual void InitializeWidget_Implementation(ARTS_Actor* InOwner);
	
	ARTS_Actor* Owner = nullptr;

	/** Returns the owner of this widget */
	UFUNCTION(BlueprintPure, Category = "RTS Widget")
	ARTS_Actor* GetModuleOwner() const;

};
