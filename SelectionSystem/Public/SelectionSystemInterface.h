#pragma once

#include "InputMappingContext.h"
#include "UObject/Interface.h"
#include "SelectionSystemInterface.generated.h"

class USelectableComponent;

UINTERFACE(MinimalAPI, Blueprintable, BlueprintType)
class USelectionSystemInterface : public UInterface
{
	GENERATED_BODY()
};

class ISelectionSystemInterface
{
	GENERATED_IINTERFACE_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Selection System Interface")
	USelectableComponent* GetSelectableComponent();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Selection System Interface")
	UInputMappingContext* GetSelectedContext();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Selection System Interface")
	UInputMappingContext* GetHoveredContext();
};
