// Copyright 2025 AmberleafCotton. All right reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SelectionTypes.h"
#include "SelectionSystemComponent.generated.h"

class UTeamComponent;
DECLARE_LOG_CATEGORY_EXTERN(LogSelectionManager, Log, All);

class UUserWidget;
class APlayerCameraManager;
class ASelectionSystemDebugger;

#pragma region STRUCTS

USTRUCT(BlueprintType)
struct FObjectSelectionSettings
{
	GENERATED_BODY()
	
	/** Allows an object to be selected by index
	* (For example - If true, all objects sharing the same index will be selected when double - clicked)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager")
		bool CanBeSelectedByIndex;

	/** Index used for selection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager")
		int32 SelectionIndex;

	/** Allows an object to be selected using the selection frame */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager")
		bool CanBeSelectedWithMarquee;

	/** Allows an object to be selected using "Select All" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager")
		bool CanBeSelectedViaSelectAll;

	/** If True, a complex collision will be used for object marquee selection.
	* Parameters for this collision should be set in the "Get Object Dynamic Collisions"
	* function in the Selection Manager Interface.
	* Slowest method, recommended to use only if you don't have many objects to select 
	* (For example - Can be used if you need precise collision for an object that can move,
	* e.g. you can tie position of collision to character�s bones)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager")
		bool UseDynamicCollisions;

	/** If true, a simple dot collision will be used, coinciding with center of the object.
	* Best performance (Useful if you have a large number of units to select)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager")
		bool UseSimpleCollision;

	/** CollisionLOD's are used for selection using a marquee selection frame.
	* Each element in the array consists of a set of Selection Spheres and Switching Distance LOD's.
	* As long as the camera-to-object distance is less than the Switching Distance LOD�s, previous LOD will be active.
	* Sphere Collision consists of Center Offset and Radius.
	* Center Offset is the offset of the collision relative to the center of the object,
	* and Radius is the radius of that collision
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager")
		AActor* Actor;
		TArray<struct FSelectCollision> CollisionLODs;
		TArray<struct FSphereCollision> LODsBounds;
		TArray<float> LODsSwitchDistances;
		TArray<bool> NeedBounds;

	//Default Constructor
	FObjectSelectionSettings(): CanBeSelectedByIndex(true),
	                            SelectionIndex(0),
	                            CanBeSelectedWithMarquee(true),
	                            CanBeSelectedViaSelectAll(true),
	                            UseDynamicCollisions(false),
	                            UseSimpleCollision(false),
								Actor(nullptr)
	{
	}
};

USTRUCT()
struct FSelectionGroup
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<AActor>> Actors;
};

#pragma endregion STRUCTS

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SELECTIONSYSTEM_API USelectionSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USelectionSystemComponent();

	/** Objects with this type of collision will be highlighted when clicked or hovered over */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager|Cursor")
		TEnumAsByte<ECollisionChannel> TraceChannel;

	/** If true, object's mesh will be taken into consideration during cursor selection
	 * If false, only the collisions of the object will be taken into consideration
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager|Cursor")
		bool bTraceComplex;

	/** If true, object under cursor will be dynamically highlighted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager|Cursor")
		bool bHighlightObjectUnderCursor;

	/** If true, when selecting the Actor and Scene Components at the same time, priority will be given to the Actor
	 * If false, priority is given to the Scene Component
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager|Cursor")
		bool ActorHitPriority;

	/** Minimum distance the cursor must move after the start of the selection for a selection frame to appear */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager|Cursor")
		float MinimumDragDistance;

	/** Widget class used for the selection frame, select WB_SM_MarqueeTool that comes with the plugin, or your own widget */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager|Marquee Tool")
		TSubclassOf<UUserWidget> MarqueeWidgetClass;

	/** Enables dynamic highlighting of objects inside the selection frame */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager|Marquee Tool")
		bool bEnableDynamicObjectHighlighting;

	/** If active, the start position of the selection frame will be tied to the place
	* where you clicked when the camera moves.
	* Allows you to select objects during camera movement
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager|Marquee Tool")
		bool bSnapMarqueeStartPosition;

	/** Should this be relative to the player viewport subregion (useful when using player attached widgets in split screen) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager|Marquee Tool")
		bool bPlayerViewportRelative;

	/** Enable this setting only if your camera has the orthographic mode enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager")
		bool bOrthographicProjection;

	/** Player controller index that is used for retrieval of a link to the player controller */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager")
		int32 PlayerControllerIndex;

	/** Debugging setting
	Enables rendering of debugging selection collisions and makes their LOD�s visible when the game is running
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager|Debug")
		bool bShowDebugCollisions;

	/** Debugging setting
	Debugger class, select BP_SM_Debugger, which is included with the plugin
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection Manager|Debug")
		TSubclassOf<ASelectionSystemDebugger> DebuggerClass;


	/** True if selection has been started using a marquee frame */
	UPROPERTY(BlueprintReadOnly, Category = "Selection Manager")
		bool bMarqueeSelectionStart;

	/** True if additive selection is enabled */
	UPROPERTY(BlueprintReadOnly, Category = "Selection Manager")
		bool bAdditionSelection;

	/** True if subtractive selection is enabled */
	UPROPERTY(BlueprintReadOnly, Category = "Selection Manager")
		bool bSubtractionSelection;




private:
	UPROPERTY()
		APlayerController* APController;
	
	UFUNCTION(BlueprintCallable)
	void CachePlayerControllerTeam(APlayerController* LocalPlayerController) const;

	ETeamAffiliation CalculateTeamAffiliation(UTeamComponent* ActorTeam) const;
	
	mutable TObjectPtr<UTeamComponent> PlayerControllerTeamComponent;
	
	UPROPERTY()
		APlayerCameraManager* ACameraManager;
	UPROPERTY()
		AActor* LastObjectUnderCursor;
	
	UPROPERTY()
		UUserWidget* MarqueeWidget;

	bool bPlayerControllerIsSet;
	bool bCameraManagerIsSet;
	bool bMarqueeWidgetIsSet;

	bool bManagerReady;
	bool bSelectionStart;
	bool bAcceptableDragDistance;
	bool bSnapMarqueeStartPositionDynamic;
	bool bSelectionByIndex;
	bool ObjectUnderCursorHighlighted;

	float MinimumMarqueeSize;

	FVector2D StartMarqueePosition;
	FVector2D FinishMarqueePosition;
	FVector2D StartMarqueePositionDynamic;
	FVector2D FinishMarqueePositionDynamic;

	FVector GroundHit;
	FVector CameraLocation;
	float ViewportScale;

	//For Marquee Selection
	FVector LTPoint;
	FVector RTPoint;
	FVector RBPoint;
	FVector LBPoint;
	FVector LTDirection;
	FVector RTDirection;
	FVector RBDirection;
	FVector LBDirection;
	FVector BotPlaneNormal;
	FVector TopPlaneNormal;
	FVector LeftPlaneNormal;
	FVector RightPlaneNormal;
	FVector DiagonalPlaneNormal;
	FVector DistanceToObject;

	//TArray<UObject*> RegisteredInterfaces;
	TArray<FObjectSelectionSettings> RegisteredActors;

	TArray<int32> GroupNumbers;
	TArray<FSelectionGroup> SelectionGroups;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


//Selection
	/** Starts selection of objects, with a frame or a click */
	UFUNCTION(BlueprintCallable, Category = "Selection Manager|Marquee Tool")
		void StartSelection();

	/** Finishes selection of objects, selects whatever is inside the frame or object below the cursor.
	* For selection to happen, selectable object must have Selection Manager Interface
	*/
	UFUNCTION(BlueprintCallable, Category = "Selection Manager|Marquee Tool")
		void FinishSelection();

	/** Enables additive selection, new selected objects will be appended to the ones already selected */
	UFUNCTION(BlueprintCallable, Category = "Selection Manager")
		void SetAdditionSelectionEnabled(bool Enabled);

	/** Enables subtractive selection, new selected objects will be deselected */
	UFUNCTION(BlueprintCallable, Category = "Selection Manager")
		void SetSubtractionSelectionEnabled(bool Enabled);

	/** Objects with indexes in the "Indexes" array will be selected */
	UFUNCTION(BlueprintCallable, Category = "Selection Manager")
		void SelectObjectsByIndexes(TArray<int32> Indexes, bool SelectionOnlyOnScreen = true);

	/** Selects all registered objects */
	UFUNCTION(BlueprintCallable, Category = "Selection Manager")
		void SelectAll(bool SelectionOnlyOnScreen);

	/** Unselect all selected units */
	UFUNCTION(BlueprintCallable, Category = "Selection Manager")
		void UnselectAll(bool IgnoreSelectionModifiers = false);

	/** Creates a selection group of currently selected and registered objects. 
	*(For example - RTS games where you press Ctrl + 1, + 2 etc. to make groups of units) 
	*/
	UFUNCTION(BlueprintCallable, Category = "Selection Manager")
		void MakeSelectionGroup(int32 GroupNumber);

	// Selects a previously created group by its number 
	UFUNCTION(BlueprintCallable, Category = "Selection Manager")
		void SelectGroup(int32 GroupNumber);
	



private:

		void FindObjectUnderCursor();

		void SelectObjectUnderCursor(AActor* Actor) const;

		// Objects under the cursor will be continuously highlighted.  Now accepts only an actor, since only actors are registered.
		void HighlightObjectUnderCursor(AActor* ActorUnderCursor);

		void ResetAllHighlighting();

		bool CheckIfObjectOnScreen(const FVector& ObjectLocation) const;
	
		void ApplySelectionState(AActor* Actor, ESelectionState DesiredState) const;


		//MARQUEE SELECTION

		/** Set starting position of marquee selection frame on the screen */
		void SetStartMarqueePositionInViewport(FVector2D Position);

		/** Set final position of the marquee selection frame on the screen */
		void SetFinishMarqueePositionInViewport(FVector2D Position);

		void UpdateMarqueeSelection();

		void SelectObjectsInMarquee();

		void HighlightObjectsInMarquee();

		void UpdateMarqueeWidget(FVector2D StartPosition, FVector2D FinishPosition) const;

		bool CheckIfActorInSelectedArea(const FTransform& ObjectTransform, FObjectSelectionSettings& ObjectSettings);

		bool CheckIfPointInSelectedArea(FVector PointLocation, float SquareRadius) const;
	
// OBJECTS REGISTRATION
	/** For the object to be selectable, it must be registered.
	* Call this function (RegisterSelectable) in the Begin Play of the selectable actor.
	* Only an Actor is accepted. The function stores the actor along with its selection settings.
	*/
	
public:
	UFUNCTION(BlueprintCallable, Category = "Selection Manager|Registration")
	void RegisterSelectable(AActor* Actor, FObjectSelectionSettings Settings);
	
	UFUNCTION(BlueprintCallable, Category = "Selection Manager|Registration")
	void UnregisterSelectable(AActor* Actor);

	
//SECONDARY FUNCTIONS
	
private:
	static void CalculateCollisionBounds(TArray<struct FSphereCollision>& Collisions, FVector& BoundsOffset, float& BoundsRadius);

	int32 GetCurrentLOD(TArray<float>& Distances, float SquaringDistanceToObject) const;

	bool ProjectWorldToScreenBidirectional(const FVector& WorldPosition, FVector2D& ScreenPosition) const;

	/**
	 * Compare Squared Distance from point projection to plane with Squared Radius.
	 *
	 * @param P Point.
	 * @param PB Plane Base.
	 * @param PN Plane Normal.
	 * @param R Squared Radius.
	 * @return true if Distance > Radius, else false.
	 */
	static bool CompareDistWithRadius(const FVector& P, const FVector& PB, const FVector& PN, const float R);

	bool CheckRightAndLeftPlanes(const FVector& PointDirection, const FVector& PointLocation, float SquareRadius) const;

	bool CheckRightAndLeftPlanesOrt(const FVector& PointLocation, float SquareRadius) const;

	static bool IsSelected(ESelectionState State);

	static bool IsHighlighted(ESelectionState State);

	void RemoveNullObjects();


//Debug
private:
	void SpawnDebugCollision(
		AActor* Actor, 
		USceneComponent* Component, 
		const TArray<struct FSelectCollision>& Collisions, 
		const bool UseDynamicCollisions);
};