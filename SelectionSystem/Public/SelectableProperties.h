#pragma once

#include "CoreMinimal.h"
#include "Components/DecalComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/PlayerController.h"
#include "SelectionSystemComponent.h"

#include "SelectableProperties.generated.h"
/**
 * Struct containing initialization properties for a selectable component.
 */
USTRUCT(BlueprintType)
struct FSelectableProperties
{
    GENERATED_BODY()

public:

    /** Decal component used to indicate the selected state. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selectable Properties")
    UDecalComponent* DSelected = nullptr;

    /** Decal component used to indicate the hovered state. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selectable Properties")
    UDecalComponent* DHovered = nullptr;

    /** Decal component used to indicate area selection. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selectable Properties")
    UDecalComponent* AreaDecalComponent = nullptr;

    /** The owning player controller for this selectable. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selectable Properties")
    APlayerController* OwningPlayerController = nullptr;

    /** Widget component used to display health or other info. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selectable Properties")
    UWidgetComponent* HealthWidgetComponent = nullptr;
};
