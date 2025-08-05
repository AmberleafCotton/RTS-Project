// Copyright 2025 AmberleafCotton. All right reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Math/Vector.h"  // For FVector
#include "SelectionTypes.generated.h"

/** 
 * Enum for team affiliation.
 * - Neutral: No team affiliation.
 * - Owned: This object is owned by a player (typically different from ally/enemy logic).
 * - Ally: Friendly unit.
 * - Enemy: Hostile unit.
 */
UENUM(BlueprintType)
enum class ETeamAffiliation : uint8 {
    Neutral UMETA(DisplayName = "Neutral"),
    Owned   UMETA(DisplayName = "Owned"),
    Ally    UMETA(DisplayName = "Ally"),
    Enemy   UMETA(DisplayName = "Enemy")
};

/** 
 * Enum for selection states.
 * - Unselected: Not selected and not hovered.
 * - SelectedAndHovered: Selected and currently hovered.
 * - Selected: Selected but not hovered.
 * - Hovered: Not selected but currently hovered.
 */
UENUM(BlueprintType)
enum class ESelectionState : uint8 {
    Unselected         UMETA(DisplayName = "Unselected"),
    Hovered            UMETA(DisplayName = "Hovered"),
    Selected           UMETA(DisplayName = "Selected"),
    SelectedAndHovered UMETA(DisplayName = "Selected and Hovered")

};

/**
 * Enum for object ownership.
 * - Neutral: The object is neutral.
 * - Owned: The object is owned by a player.
 */
UENUM(BlueprintType)
enum class EOwnershipType : uint8 {
    Neutral UMETA(DisplayName = "Neutral"),
    Owned   UMETA(DisplayName = "Owned")
};

/**
 * Simple collision sphere structure for marquee selection.
 * Use this to define collision bounds on your SelectableComponent.
 */
USTRUCT(BlueprintType)
struct FSphereCollision
{
    GENERATED_BODY()

    /** The offset from the object's origin (usually its pivot) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FVector CenterOffset;

    /** The radius of the collision sphere */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float Radius;

    FSphereCollision()
        : CenterOffset(FVector::ZeroVector)
        , Radius(100.0f) // A good starting value for a unit centered collision
    {}
    
};

USTRUCT(BlueprintType)
struct FSelectCollision
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager")
    TArray<struct FSphereCollision> Collisions;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager")
    float SwitchingDistanceLOD;

    //Default Constructor
    FSelectCollision(): SwitchingDistanceLOD(0)
    {
    }
};