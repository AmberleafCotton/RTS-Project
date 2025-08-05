// Copyright 2025 AmberleafCotton. All right reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SelectionSystemDebugger.generated.h"

class USelectionSystemComponent;

//
// We now assume that the debugger works only with actors (not components).
// Therefore, any reference to a component owner has been removed.
//

UCLASS(Blueprintable, BlueprintType)
class SELECTIONSYSTEM_API ASelectionSystemDebugger : public AActor
{
    GENERATED_BODY()
    
public:
    // Sets default values for this actor's properties
    ASelectionSystemDebugger();

    /** Enables display of collisions in the editor so that they can be adjusted visually */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEditorMode;

    /** Enables display of boundary collisions if they were generated for this LOD */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bShowLODsBounds;

    /** If bEditorMode is enabled, set the number of the LOD so that it can be displayed in the editor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 ShowLODNumber;

    /** 
     * If bEditorMode is enabled, add new items to this array to create new selection collision elements.
     * When the collision is created, copy the value of this variable with RMB and paste it into
     * the collision variable of the object you want to select.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TArray<struct FSelectCollision> PreviewSelectionCollisions;

    /** Mesh used for the display of collision */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Debug")
    UStaticMesh* DebugMesh;

    /** Material used for the display of collision */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Debug")
    UMaterialInterface* DebugMeshMaterial;

    /** Material used for the Dot Collision */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Debug")
    UMaterialInterface* DebugDotMaterial;

    /** Material used for the boundary collision */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Debug")
    UMaterialInterface* DebugBoundsMaterial;

private:
    UPROPERTY()
    APlayerCameraManager* CameraManager;
    
    UPROPERTY()
    USelectionSystemComponent* SelectionManager;

    UPROPERTY()
    AActor* ActorOwner;
    
    UPROPERTY()
    TArray<UStaticMeshComponent*> CollisionMeshes;
    
    UPROPERTY()
    TArray<UStaticMeshComponent*> BoundsMeshes;

    UPROPERTY()
    TArray<int32> CollisionLODs;
    
    UPROPERTY()
    TArray<float> DistancesLODsSwitch;
    
    // These arrays are used for collision visualization.
    TArray<struct FSelectCollision> SelectionCollisions;
    TArray<struct FSphereCollision> SelectionCollisionLODsBounds;
    
    UPROPERTY()
    TArray<bool> SimpleBounds;
    
    TArray<FSelectCollision> DynamicCollisions;

    int32 ActiveLOD;
    int32 NewLOD;

    FVector OwnerLocation;

    bool bIsActorOwner;  // May be used to indicate if ActorOwner is valid.
    bool bDebuggerReady;
    bool bUseDynamicCollisions;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called when the actor is constructed (in the editor or at runtime).
    virtual void OnConstruction(const FTransform& Transform) override;

public:    
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    /**
     * Sets up the debugger for a given actor.
     *
     * @param PlayerCameraManager - The camera manager to use for projection.
     * @param SelectionManagerComponent - The selection manager component.
     * @param Actor - The actor that owns the collision data.
     * @param Collisions - The collision data to display.
     * @param UseDynamicCollisions - Whether to use dynamic collision data.
     */

    
    // Updated declaration in the header:
    UFUNCTION(BlueprintCallable, Category = "Selection Manager|Debug")
    void SetupDebugger(APlayerCameraManager* PlayerCameraManager,
                       USelectionSystemComponent* SelectionManagerComponent,
                       AActor* Actor,
                       const TArray<FSelectCollision>& Collisions,
                       bool UseDynamicCollisions);


private:
    void SpawnCollisions(TArray<struct FSelectCollision>& Collisions);
    void CalculateBounds(TArray<struct FSphereCollision>& Collisions);
    void ShowLODByIndex(int32 Index);
    void SwitchLODsEditorMode();
    void MoveDynamicCollisionLOD(int32 Index);
};
