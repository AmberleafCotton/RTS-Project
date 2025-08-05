// Copyright 2025 AmberleafCotton. All rights reserved.
#pragma once

#include "Components/ActorComponent.h"
#include "SelectionTypes.h"
#include "SelectionSystemComponent.h"
#include "SelectableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectionStateUpdate, ESelectionState, NewState);

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SELECTIONSYSTEM_API USelectableComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USelectableComponent();

    //~ Delegates
    UPROPERTY(BlueprintAssignable, Category = "Selectable Component")
    FOnSelectionStateUpdate OnSelectionStateUpdate;
    
    //~ Core Functions
    UFUNCTION(BlueprintCallable, Category = "Selectable Component")
    void BroadcastSelectionState(ESelectionState NewState);
    
    //~ State Accessors
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Selectable Component")
    ESelectionState GetObjectSelection() const { return CurrentSelectionState; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Selectable Component")
    FSelectCollision GetObjectDynamicCollisions() const { return DynamicCollisions; }
    
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Selectable Component")
    void UpdateActorSelection(ESelectionState NewState, ETeamAffiliation NewTeamAffiliation);

    //~ Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selectable Component")
    EOwnershipType OwnershipType;
    
    UPROPERTY(BlueprintReadWrite, Category = "Selectable Component")
    ESelectionState CurrentSelectionState;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selectable Component")
    FSelectCollision DynamicCollisions;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selectable Component")
    FObjectSelectionSettings SelectionSettings;
    
protected:
    virtual void BeginPlay() override;
};
