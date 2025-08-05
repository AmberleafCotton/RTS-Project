// Copyright 2025 AmberleafCotton. All rights reserved.

#include "SelectionSystemComponent.h"
#include "SelectionSystemDebugger.h"
#include "Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Math/Vector2D.h"
#include "SceneView.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameViewportClient.h"
#include "SelectionTypes.h"
#include "SelectableComponent.h"
#include "TeamComponent/TeamComponent.h"
#include "Components/PrimitiveComponent.h"

DEFINE_LOG_CATEGORY(LogSelectionManager);

FHitResult HitResultUnderCursor;

// SET DEFAULT VALUES
USelectionSystemComponent::USelectionSystemComponent()
{
	bManagerReady = false;
	PrimaryComponentTick.bCanEverTick = true;
	PlayerControllerIndex = 0;
	MinimumDragDistance = 20.0f;
	MinimumMarqueeSize = 8.0f;
	bSnapMarqueeStartPosition = false;
	bEnableDynamicObjectHighlighting = true;
	bTraceComplex = true;
	bHighlightObjectUnderCursor = true;
	ActorHitPriority = true;
	TraceChannel = ECollisionChannel::ECC_Visibility;
}

// BEGIN PLAY
void USelectionSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	if(!GetWorld())
	{
		bManagerReady = false;
		UE_LOG(LogSelectionManager, Warning, TEXT("Selection Manager is not ready: World is not valid"));
		return;
	}


	APController = Cast<APlayerController>(GetOwner());

	if (APController)
	{
		bPlayerControllerIsSet = true;

		ACameraManager = APController->PlayerCameraManager;

		if (ACameraManager)	bCameraManagerIsSet = true;
		else 
		{
			bCameraManagerIsSet = false;
			UE_LOG(LogSelectionManager, Warning, TEXT("Selection Manager is not ready: Camera Manager is not valid"));
		}
	}

	else
	{
		bPlayerControllerIsSet = false;
		UE_LOG(LogSelectionManager, Warning, TEXT("Selection Manager is not ready: Player Controller is not valid"));
	}


	//Create Widgets
	if (MarqueeWidgetClass != nullptr)
	{
		MarqueeWidget = CreateWidget(GetWorld(), MarqueeWidgetClass, "MarqueeWidget");
		MarqueeWidget->SetVisibility(ESlateVisibility::Collapsed);
		MarqueeWidget->AddToViewport();
		bMarqueeWidgetIsSet = true;
	}

	else
	{
		bMarqueeWidgetIsSet = false;
		UE_LOG(LogSelectionManager, Warning, TEXT("Selection Manager is not ready: Marquee Widget is not valid"));
	}


	bManagerReady = bPlayerControllerIsSet && bCameraManagerIsSet && bMarqueeWidgetIsSet;
}


// TICK
void USelectionSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bManagerReady) return;

	if (bHighlightObjectUnderCursor) FindObjectUnderCursor();

	if (!bSelectionStart) return;

	FVector2D MousePosition;
	APController->GetMousePosition(MousePosition.X, MousePosition.Y);
	SetFinishMarqueePositionInViewport(MousePosition);

	if (!bAcceptableDragDistance)
	{
		const FVector2D DragVector = FinishMarqueePosition - StartMarqueePosition;
		const float DragDistance = DragVector.Size();

		if (DragDistance > MinimumDragDistance) {
			bAcceptableDragDistance = true;
			bMarqueeSelectionStart = true;
		}

		return;
	}

	CameraLocation = ACameraManager->GetCameraLocation();

	//Set main Marquee frame and parameters
	UpdateMarqueeSelection();

	if (bEnableDynamicObjectHighlighting)
	{
		HighlightObjectsInMarquee();
	}
}

// SELECTION
void USelectionSystemComponent::StartSelection()
{
	// If the manager isn’t ready, exit early.
	if (!bManagerReady)
		return;

	// If we want to "snap" the marquee start position dynamically (i.e. tie it to a world hit),
	// perform a hit test under the cursor.
	if (bSnapMarqueeStartPosition)
	{
		// Perform a hit test using the configured trace channel and complexity.
		if (APController->GetHitResultUnderCursor(TraceChannel, bTraceComplex, HitResultUnderCursor))
		{
			// If a blocking hit is found, store the hit location as the "ground hit".
			GroundHit = HitResultUnderCursor.Location;
			bSnapMarqueeStartPositionDynamic = true;
		}
		else
		{
			bSnapMarqueeStartPositionDynamic = false;
		}
	}
	else
	{
		bSnapMarqueeStartPositionDynamic = false;
	}

	// Clear any current selection.
	UnselectAll();

	// Set the flag indicating that a selection (marquee) has started.
	bSelectionStart = true;

	// Retrieve the current mouse position from the PlayerController.
	FVector2D MousePosition;
	APController->GetMousePosition(MousePosition.X, MousePosition.Y);

	// Set the starting position of the marquee selection using the current mouse position.
	SetStartMarqueePositionInViewport(MousePosition);
}


void USelectionSystemComponent::FinishSelection()
{
	if (!(bManagerReady && bSelectionStart))
		return;

	if (bMarqueeSelectionStart)
	{
		MarqueeWidget->SetVisibility(ESlateVisibility::Collapsed);
		SelectObjectsInMarquee();
	}
	else 
	{
		if (APController->GetHitResultUnderCursor(TraceChannel, bTraceComplex, HitResultUnderCursor))
		{
			// Retrieve the actor directly from the hit result.
			AActor* HitActor = HitResultUnderCursor.GetActor();
			if (HitActor)
			{
				// Use the actor directly; no need to check for a component.
				SelectObjectUnderCursor(HitActor);
			}
		}
	}

	bSelectionStart = false;
	bAcceptableDragDistance = false;
	bMarqueeSelectionStart = false;

	RemoveNullObjects();
}


void USelectionSystemComponent::FindObjectUnderCursor()
{
	// Perform a hit test under the cursor.
	if (!APController->GetHitResultUnderCursor(TraceChannel, bTraceComplex, HitResultUnderCursor))
		return;

	if (!HitResultUnderCursor.bBlockingHit)
		return;

	// Retrieve the actor from the hit result.
	AActor* HitActor = HitResultUnderCursor.GetActor();
	if (!HitActor)
		return;

	// Since we register only actors, simply highlight the hit actor.
	HighlightObjectUnderCursor(HitActor);
}


void USelectionSystemComponent::SetAdditionSelectionEnabled(const bool Enabled)
{
	if (bAdditionSelection == Enabled) return;

	if (bMarqueeSelectionStart) ResetAllHighlighting();

	bAdditionSelection = Enabled;
}

void USelectionSystemComponent::SetSubtractionSelectionEnabled(const bool Enabled)
{
	if (bSubtractionSelection == Enabled) return;

	if (bMarqueeSelectionStart) ResetAllHighlighting();

	bSubtractionSelection = Enabled;
}

void USelectionSystemComponent::SelectObjectsByIndexes(const TArray<int32> Indexes, const bool SelectionOnlyOnScreen)
{
	if (Indexes.Num() <= 0)
	{
		return;
	}

	UnselectAll();

	// Iterate over all Registered Actors.
	for (int i = 0; i < RegisteredActors.Num(); i++)
	{
		if (!RegisteredActors[i].CanBeSelectedByIndex)
			continue;
		if (Indexes.Find(RegisteredActors[i].SelectionIndex) < 0)
			continue;

		// Since only actors are registered, retrieve the actor directly.
		AActor* Actor = RegisteredActors[i].Actor;
		if (!Actor)
			continue;

		// Instead of calling UpdateObjectSelection directly, use ApplySelectionState.
		if (!SelectionOnlyOnScreen)
		{
			if (!bSubtractionSelection || bAdditionSelection)
				ApplySelectionState(Actor, ESelectionState::SelectedAndHovered);
			else
				ApplySelectionState(Actor, ESelectionState::Unselected);
			continue;
		}

		// Since we register only actors, we always use the actor's location.
		FVector ObjectLocation = Actor->GetActorLocation();

		if (CheckIfObjectOnScreen(ObjectLocation))
		{
			if (!bSubtractionSelection || bAdditionSelection)
				ApplySelectionState(Actor, ESelectionState::SelectedAndHovered);
		}
	}
}


void USelectionSystemComponent::HighlightObjectUnderCursor(AActor* ActorUnderCursor)
{
    if (!ActorUnderCursor)
        return;

    // When marquee selection is active, do not change highlighting.
    if (bMarqueeSelectionStart)
    {
        if (!ObjectUnderCursorHighlighted)
            return;

        // Reset highlighting on the last actor.
        if (LastObjectUnderCursor)
        {
            if (USelectableComponent* LastSelComp = LastObjectUnderCursor->FindComponentByClass<USelectableComponent>())
            {
                // If the actor was selected, revert to Selected (unhovered); otherwise, unselect.
                ESelectionState LastState = LastSelComp->GetObjectSelection();
                if (IsSelected(LastState))
                {
                    ApplySelectionState(LastObjectUnderCursor, ESelectionState::Selected);
                }
                else
                {
                    ApplySelectionState(LastObjectUnderCursor, ESelectionState::Unselected);
                }
                ObjectUnderCursorHighlighted = false;
            }
        }
        return;
    }

    // If the new actor under the cursor is the same as the last one, do nothing.
    if (LastObjectUnderCursor == ActorUnderCursor)
        return;

    // Reset previous actor's highlighting if needed.
    if (LastObjectUnderCursor && ObjectUnderCursorHighlighted)
    {
        if (USelectableComponent* LastSelComp = LastObjectUnderCursor->FindComponentByClass<USelectableComponent>())
        {
            ESelectionState LastState = LastSelComp->GetObjectSelection();
            if (IsSelected(LastState))
            {
                ApplySelectionState(LastObjectUnderCursor, ESelectionState::Selected);
            }
            else
            {
                ApplySelectionState(LastObjectUnderCursor, ESelectionState::Unselected);
            }
            ObjectUnderCursorHighlighted = false;
        }
    }

    LastObjectUnderCursor = ActorUnderCursor;

    USelectableComponent* SelComp = ActorUnderCursor->FindComponentByClass<USelectableComponent>();
    if (!SelComp)
        return;

    ESelectionState CurrentState = SelComp->GetObjectSelection();
    // If the actor is already selected, set it to "SelectedAndHovered".
    // Otherwise, if it is not selected, set it to "Hovered".
    if (IsSelected(CurrentState))
    {
        ApplySelectionState(ActorUnderCursor, ESelectionState::SelectedAndHovered);
    }
    else if (!IsHighlighted(CurrentState))
    {
        ApplySelectionState(ActorUnderCursor, ESelectionState::Hovered);
    }
    ObjectUnderCursorHighlighted = true;
}


void USelectionSystemComponent::SelectAll(const bool SelectionOnlyOnScreen)
{
	UnselectAll(true);

	// Iterate over all registered actors (only actors are registered now)
	for (int i = 0; i < RegisteredActors.Num(); i++)
	{
		// Skip if the actor is not selectable via "Select All".
		if (!RegisteredActors[i].CanBeSelectedViaSelectAll)
			continue;

		// Retrieve the actor directly from the registration data.
		AActor* Actor = RegisteredActors[i].Actor;
		if (!Actor)
			continue;

		// Retrieve the SelectableComponent from the actor.
		USelectableComponent* SelComp = Actor->FindComponentByClass<USelectableComponent>();
		if (!SelComp)
			continue;

		// If SelectionOnlyOnScreen is enabled, check if the actor is on screen.
		if (SelectionOnlyOnScreen)
		{
			FVector ObjectLocation = Actor->GetActorLocation();
			if (CheckIfObjectOnScreen(ObjectLocation))
			{
				ApplySelectionState(Actor, ESelectionState::SelectedAndHovered);
			}
		}
		else
		{
			ApplySelectionState(Actor, ESelectionState::SelectedAndHovered);
		}
	}
}



void USelectionSystemComponent::MakeSelectionGroup(const int32 GroupNumber)
{
	int32 Index = GroupNumbers.Find(GroupNumber);

	if (Index >= 0)
	{
		// Clear the Actors array instead of the old Objects array.
		SelectionGroups[Index].Actors.Empty();
	}
	else
	{
		SelectionGroups.AddDefaulted();
		Index = SelectionGroups.Num() - 1;
		GroupNumbers.Add(GroupNumber);
	}

	// Iterate over all registered actors.
	for (int i = 0; i < RegisteredActors.Num(); i++)
	{
		// Retrieve the actor directly from the registration data.
		AActor* Actor = RegisteredActors[i].Actor;
		if (!Actor)
			continue;

		// Retrieve the SelectableComponent from the actor.
		USelectableComponent* SelComp = Actor->FindComponentByClass<USelectableComponent>();
		if (!SelComp)
			continue;

		// Retrieve the current selection state from the component.
		const ESelectionState State = SelComp->GetObjectSelection();

		// If the actor is selected (using our helper IsSelected function), add it to the group.
		if (IsSelected(State))
		{
			SelectionGroups[Index].Actors.Add(Actor);
		}
	}
}

void USelectionSystemComponent::SelectGroup(const int32 GroupNumber)
{
	// Find the group index for the given GroupNumber.
	const int32 Index = GroupNumbers.Find(GroupNumber);
	if (Index < 0)
		return;

	// Clear any existing selection.
	UnselectAll(true);

	// Iterate over all actors in the selection group.
	for (int i = 0; i < SelectionGroups[Index].Actors.Num(); i++)
	{
		AActor* Actor = SelectionGroups[Index].Actors[i];
		if (Actor)
		{
			// Instead of directly calling UpdateObjectSelection,
			// use the helper function to apply the team settings check
			// and update the selection state.
			ApplySelectionState(Actor, ESelectionState::SelectedAndHovered);
		}
	}
}



void USelectionSystemComponent::SelectObjectUnderCursor(AActor* Actor) const
{
	if (!Actor)
		return;
    
	// Determine the desired selection state based on whether subtraction selection is active.
	ESelectionState DesiredState = bSubtractionSelection ? ESelectionState::Unselected : ESelectionState::SelectedAndHovered;

	// Use the helper function to apply the team and ownership checks before updating selection.
	ApplySelectionState(Actor, DesiredState);
}


void USelectionSystemComponent::UnselectAll(bool IgnoreSelectionModifiers)
{
	if (!IgnoreSelectionModifiers)
	{
		if (bAdditionSelection || bSubtractionSelection)
			return;
	}

	for (int i = 0; i < RegisteredActors.Num(); i++)
	{
		// Retrieve the actor directly from the registration (we now register only actors)
		AActor* Actor = RegisteredActors[i].Actor;
		if (!Actor)
			continue;

		// Instead of directly calling UpdateObjectSelection,
		// use ApplySelectionState to perform the team-affiliation check and then unselect.
		ApplySelectionState(Actor, ESelectionState::Unselected);
	}
}


void USelectionSystemComponent::ResetAllHighlighting()
{
	for (int i = 0; i < RegisteredActors.Num(); i++)
	{
		// Retrieve the actor directly (since we register only actors).
		AActor* Actor = RegisteredActors[i].Actor;
		if (!Actor)
			continue;

		// Retrieve the SelectableComponent from the actor.
		USelectableComponent* SelComp = Actor->FindComponentByClass<USelectableComponent>();
		if (!SelComp)
			continue;

		// Get the current selection state from the component.
		ESelectionState State = SelComp->GetObjectSelection();

		// If the object is selected, mark it as SelectedAndHovered; otherwise, mark it as Unselected.
		if (IsSelected(State))
			ApplySelectionState(Actor, ESelectionState::SelectedAndHovered);
		else
			ApplySelectionState(Actor, ESelectionState::Unselected);
	}
}


bool USelectionSystemComponent::CheckIfObjectOnScreen(const FVector& ObjectLocation) const
{
	FVector2D ScreenLocation;

	if (!APController->ProjectWorldLocationToScreen(ObjectLocation, ScreenLocation, bPlayerViewportRelative)) return false;

	constexpr float Offset = 20.0f;

	int32 SizeX = 0;
	int32 SizeY = 0;

	APController->GetViewportSize(SizeX, SizeY);

	const float ScreenSizeX = SizeX;
	const float ScreenSizeY = SizeY;

	if (ScreenLocation.X > (Offset * -1) && ScreenLocation.X < ScreenSizeX + Offset &&
		ScreenLocation.Y > (Offset * -1) && ScreenLocation.Y < ScreenSizeY + Offset)
	{
		return true;
	}

	return false;
}


// MARQUEE SELECTION
//START
void USelectionSystemComponent::SetStartMarqueePositionInViewport(FVector2D Position)
{
	StartMarqueePosition = Position;
}


//FINISH
void USelectionSystemComponent::SetFinishMarqueePositionInViewport(FVector2D Position)
{
	FinishMarqueePosition = Position;
}


void USelectionSystemComponent::UpdateMarqueeSelection()
{
	FVector2D CurrentPosition;
	ViewportScale = UWidgetLayoutLibrary::GetViewportScale(GetWorld());

	if (bSnapMarqueeStartPositionDynamic)
	{
		ProjectWorldToScreenBidirectional(GroundHit, CurrentPosition);
	}

	else
	{
		CurrentPosition = StartMarqueePosition;
	}


	if (CurrentPosition.X < FinishMarqueePosition.X){
		StartMarqueePositionDynamic.X = CurrentPosition.X;
		FinishMarqueePositionDynamic.X = FinishMarqueePosition.X;
	}
	else { 
		StartMarqueePositionDynamic.X = FinishMarqueePosition.X;
		FinishMarqueePositionDynamic.X = CurrentPosition.X;
	}

	if (CurrentPosition.Y < FinishMarqueePosition.Y){
		StartMarqueePositionDynamic.Y = CurrentPosition.Y;
		FinishMarqueePositionDynamic.Y = FinishMarqueePosition.Y;
	}
	else {
		StartMarqueePositionDynamic.Y = FinishMarqueePosition.Y;
		FinishMarqueePositionDynamic.Y = CurrentPosition.Y;
	}

	//Set visual Marquee size and position
	UpdateMarqueeWidget(StartMarqueePositionDynamic, FinishMarqueePositionDynamic);

	if (FinishMarqueePositionDynamic.X - StartMarqueePositionDynamic.X < MinimumMarqueeSize) {
		CurrentPosition.X = StartMarqueePositionDynamic.X + MinimumMarqueeSize;
	}
	else {
		CurrentPosition.X = FinishMarqueePositionDynamic.X;
	}

	if (FinishMarqueePositionDynamic.Y - StartMarqueePositionDynamic.Y < MinimumMarqueeSize) {
		CurrentPosition.Y = StartMarqueePositionDynamic.Y + MinimumMarqueeSize;
	}
	else {
		CurrentPosition.Y = FinishMarqueePositionDynamic.Y;
	}

	FinishMarqueePositionDynamic = CurrentPosition;


	//Setup main values for check every object
	//bOrthographicProjection = ACameraManager->IsOrthographic(); //not work, always false

	APController->DeprojectScreenPositionToWorld(
		StartMarqueePositionDynamic.X,
		StartMarqueePositionDynamic.Y,
		LTPoint, LTDirection);

	APController->DeprojectScreenPositionToWorld(
		FinishMarqueePositionDynamic.X,
		StartMarqueePositionDynamic.Y,
		RTPoint, RTDirection);

	APController->DeprojectScreenPositionToWorld(
		FinishMarqueePositionDynamic.X,
		FinishMarqueePositionDynamic.Y,
		RBPoint, RBDirection);

	APController->DeprojectScreenPositionToWorld(
		StartMarqueePositionDynamic.X,
		FinishMarqueePositionDynamic.Y,
		LBPoint, LBDirection);

	if (bOrthographicProjection)
	{
		BotPlaneNormal = UKismetMathLibrary::GetUpVector(ACameraManager->GetCameraRotation());
		TopPlaneNormal = BotPlaneNormal * -1;
		LeftPlaneNormal = UKismetMathLibrary::GetRightVector(ACameraManager->GetCameraRotation());
		RightPlaneNormal = LeftPlaneNormal * -1;
	}
	else
	{
		BotPlaneNormal = (LBPoint - CameraLocation) ^ (RBPoint - CameraLocation);
		BotPlaneNormal.Normalize();

		TopPlaneNormal = (LTPoint - CameraLocation) ^ (RTPoint - CameraLocation) * -1;
		TopPlaneNormal.Normalize();

		LeftPlaneNormal = (LTPoint - CameraLocation) ^ (LBPoint - CameraLocation);
		LeftPlaneNormal.Normalize();

		RightPlaneNormal = (RTPoint - CameraLocation) ^ (RBPoint - CameraLocation) * -1;
		RightPlaneNormal.Normalize();

		DiagonalPlaneNormal = (RBPoint - CameraLocation) ^ (LTPoint - CameraLocation);
		DiagonalPlaneNormal.Normalize();
	}
}


bool USelectionSystemComponent::CheckIfActorInSelectedArea(const FTransform& ObjectTransform, FObjectSelectionSettings& ObjectSettings)
{
    // Simple Collision
    if (ObjectSettings.UseSimpleCollision)
    {
        return CheckIfPointInSelectedArea(ObjectTransform.GetLocation(), 0.0f);
    }

    // Squared Distance To Object
    const float SquaringDistanceToObject = 
        UKismetMathLibrary::Vector_DistanceSquared(ObjectTransform.GetLocation(), CameraLocation);

	// Dynamic Collision
	if (ObjectSettings.UseDynamicCollisions)
	{
		// Since only actors are registered, directly retrieve the actor.
		AActor* Actor = ObjectSettings.Actor;
		// (Assuming Actor is guaranteed to be non-null by the registration system)
		float Scale = Actor->GetActorScale().X;

		// Retrieve dynamic collisions directly from the actor's SelectableComponent.
		USelectableComponent* SelComp = Actor->FindComponentByClass<USelectableComponent>();
		if (!SelComp) 
		{
			// Optionally log a warning if an actor is missing the component.
			return false;
		}
		FSelectCollision CollisionLODs = SelComp->GetObjectDynamicCollisions();

		Scale *= Scale;

		for (int i = 0; i < CollisionLODs.Collisions.Num(); i++)
		{
			float Radius = CollisionLODs.Collisions[i].Radius;
			Radius *= Scale;

			if (CheckIfPointInSelectedArea(CollisionLODs.Collisions[i].CenterOffset, Radius))
				return true;
		}

		return false;
	}


    // Complex Collision
    const int32 CurrentLOD = GetCurrentLOD(ObjectSettings.LODsSwitchDistances, SquaringDistanceToObject);
    
    FVector BoundsLocation;
    float Radius;

    // If bounds are needed and selected, check the currently active LOD collisions.
    if (ObjectSettings.NeedBounds[CurrentLOD])
    {
        if (ObjectSettings.LODsBounds[CurrentLOD].CenterOffset.Equals(FVector::ZeroVector))
            BoundsLocation = ObjectTransform.GetLocation();
        else
            BoundsLocation = UKismetMathLibrary::TransformLocation(ObjectTransform, ObjectSettings.LODsBounds[CurrentLOD].CenterOffset);
        
        Radius = ObjectSettings.LODsBounds[CurrentLOD].Radius * ObjectTransform.GetScale3D().X * ObjectTransform.GetScale3D().X;

        if (!CheckIfPointInSelectedArea(BoundsLocation, Radius))
            return false;
    }

    FVector CollisionLocation;

    // Check Sphere Collisions
    for (int i = 0; i < ObjectSettings.CollisionLODs[CurrentLOD].Collisions.Num(); i++)
    {
        if (ObjectSettings.CollisionLODs[CurrentLOD].Collisions[i].CenterOffset.Equals(FVector::ZeroVector))
            CollisionLocation = ObjectTransform.GetLocation();
        else
            CollisionLocation = UKismetMathLibrary::TransformLocation(ObjectTransform, ObjectSettings.CollisionLODs[CurrentLOD].Collisions[i].CenterOffset);
            
        Radius = ObjectSettings.CollisionLODs[CurrentLOD].Collisions[i].Radius * ObjectTransform.GetScale3D().X * ObjectTransform.GetScale3D().X;

        if (CheckIfPointInSelectedArea(CollisionLocation, Radius))
            return true;
    }

    return false;
}


bool USelectionSystemComponent::CheckIfPointInSelectedArea(const FVector PointLocation, const float SquareRadius) const
{
	const bool DotCollision = SquareRadius < 0.01;

	//Orthographic Projection
	if (bOrthographicProjection)
	{
		//Dot Collision
		if (DotCollision)
		{
			if (FVector::DotProduct(BotPlaneNormal, PointLocation - RBPoint) < 0.0) return false;
			if (FVector::DotProduct(LeftPlaneNormal, PointLocation - LTPoint) < 0.0) return false;
			if (FVector::DotProduct(RightPlaneNormal, PointLocation - RTPoint) < 0.0) return false;
			return FVector::DotProduct(TopPlaneNormal, PointLocation - LTPoint) > 0.0;
		}


		//Sphere Collision
		//Check which side point is from planes
		if (FVector::DotProduct(TopPlaneNormal, PointLocation - LTPoint) > 0.0)
		{
			if (FVector::DotProduct(BotPlaneNormal, PointLocation - RBPoint) > 0.0)
				return CheckRightAndLeftPlanesOrt(PointLocation, SquareRadius);

			if (CompareDistWithRadius(PointLocation, LBPoint, BotPlaneNormal, SquareRadius)) return false;

			return CheckRightAndLeftPlanesOrt(PointLocation, SquareRadius);
		}

		if (CompareDistWithRadius(PointLocation, RTPoint, TopPlaneNormal, SquareRadius)) return false;

		return CheckRightAndLeftPlanesOrt(PointLocation, SquareRadius);
	}
	
	//Non Orthographic Projection Settings
	const FVector PointDirection = PointLocation - CameraLocation;

	//Dot Collision
	if (DotCollision)
	{
		//Check if there is point in the left or right pyramid
		if (FVector::DotProduct(DiagonalPlaneNormal, PointDirection) > 0.0)
		{
			//Check which side point is from Bot plane
			if (FVector::DotProduct(BotPlaneNormal, PointDirection) > 0.0)
			{
				//Check which side point is from Left plane
				return (FVector::DotProduct(LeftPlaneNormal, PointDirection) > 0.0);
			}
			return false;
		}

		//Check which side point is from Top plane
		if (FVector::DotProduct(TopPlaneNormal, PointDirection) > 0.0)
		{
			//Check which side point is from Right plane
			return (FVector::DotProduct(RightPlaneNormal, PointDirection) > 0.0);
		}

		return false;
	}


	//Sphere Collision
	//Check which side point is from planes
	if(FVector::DotProduct(TopPlaneNormal, PointDirection) > 0.0)
	{
		if (FVector::DotProduct(BotPlaneNormal, PointDirection) > 0.0)
			return CheckRightAndLeftPlanes(PointDirection, PointLocation, SquareRadius);

		if (CompareDistWithRadius(PointLocation, CameraLocation, BotPlaneNormal, SquareRadius)) return false;

		return CheckRightAndLeftPlanes(PointDirection, PointLocation, SquareRadius);
	}

	if (CompareDistWithRadius(PointLocation, CameraLocation, TopPlaneNormal, SquareRadius)) return false;

	return CheckRightAndLeftPlanes(PointDirection, PointLocation, SquareRadius);
}


void USelectionSystemComponent::SelectObjectsInMarquee()
{
	// If neither addition nor subtraction selection is active, clear any previous selection.
	if (!(bAdditionSelection || bSubtractionSelection))
	{
		UnselectAll();
	}

	// Enumerate over all registered objects (now assumed to be actors only)
	for (int i = 0; i < RegisteredActors.Num(); i++)
	{
		// Skip if object cannot be selected with marquee.
		if (!RegisteredActors[i].CanBeSelectedWithMarquee)
			continue;

		// Retrieve the actor (registration now stores only actors).
		AActor* Actor = RegisteredActors[i].Actor;
		if (!Actor)
			continue;

		// Get the actor's transform.
		FTransform ObjectTransform = Actor->GetActorTransform();

		// Retrieve the SelectableComponent from the actor.
		USelectableComponent* SelComp = Actor->FindComponentByClass<USelectableComponent>();
		if (!SelComp)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s does not have a USelectableComponent."), *Actor->GetName());
			continue;
		}

		// Standard Selection (and Addition Selection when not subtraction)
		if (!bSubtractionSelection)
		{
			if (CheckIfActorInSelectedArea(ObjectTransform, RegisteredActors[i]))
			{
				// When inside the marquee, set the state to SelectedAndHovered.
				ApplySelectionState(Actor, ESelectionState::SelectedAndHovered);
				continue;
			}
		}

		// Subtraction Selection: if the object is inside, mark it as Unselected.
		if (bSubtractionSelection)
		{
			if (CheckIfActorInSelectedArea(ObjectTransform, RegisteredActors[i]))
			{
				ApplySelectionState(Actor, ESelectionState::Unselected);
			}
		}
	}
}


void USelectionSystemComponent::HighlightObjectsInMarquee()
{
    // Enumerate over all registered objects (which now are always actors).
    for (int i = 0; i < RegisteredActors.Num(); i++)
    {
        // Skip if object cannot be selected with marquee.
        if (!RegisteredActors[i].CanBeSelectedWithMarquee)
            continue;

        // Retrieve the actor from registration.
        AActor* Actor = RegisteredActors[i].Actor;
        if (!Actor)
            continue;

        // Retrieve the object's transform.
        FTransform ObjectTransform = Actor->GetActorTransform();

        // Retrieve the selectable component from the actor.
        USelectableComponent* SelComp = Actor->FindComponentByClass<USelectableComponent>();
        if (!SelComp)
        {
            UE_LOG(LogTemp, Warning, TEXT("%s does not have a USelectableComponent."), *Actor->GetName());
            continue;
        }

        // Get the current selection state from the component.
        ESelectionState State = SelComp->GetObjectSelection();
        bool ObjectSelected = IsSelected(State);
        bool ObjectHighlighted = IsHighlighted(State);

        // Standard Selection: when neither addition nor subtraction selection is active.
        if (!(bAdditionSelection || bSubtractionSelection))
        {
            if (CheckIfActorInSelectedArea(ObjectTransform, RegisteredActors[i]))
            {
                // If object is inside the marquee:
                // - If already selected, set to SelectedAndHovered.
                // - Otherwise, set to Hovered.
                ESelectionState NewState = ObjectSelected ? ESelectionState::SelectedAndHovered : ESelectionState::Hovered;
                if (!ObjectHighlighted)
                {
                    ApplySelectionState(Actor, NewState);
                }
            }
            else
            {
                // If object is not inside the marquee:
                // - If it was previously selected, set to Selected.
                // - Otherwise, set to Unselected.
                ESelectionState NewState = ObjectSelected ? ESelectionState::Selected : ESelectionState::Unselected;
                if (ObjectHighlighted)
                {
                    ApplySelectionState(Actor, NewState);
                }
            }
            continue;
        }

        // Addition Selection: when bAdditionSelection is true.
        if (bAdditionSelection)
        {
            if (ObjectSelected)
                continue;

            if (CheckIfActorInSelectedArea(ObjectTransform, RegisteredActors[i]))
            {
                if (!ObjectHighlighted)
                    ApplySelectionState(Actor, ESelectionState::Hovered);
            }
            else
            {
                if (ObjectHighlighted)
                    ApplySelectionState(Actor, ESelectionState::Unselected);
            }
            continue;
        }

        // Subtraction Selection: when bSubtractionSelection is true.
        if (bSubtractionSelection)
        {
            if (!ObjectSelected)
                continue;

            if (CheckIfActorInSelectedArea(ObjectTransform, RegisteredActors[i]))
            {
                if (ObjectHighlighted)
                    ApplySelectionState(Actor, ESelectionState::Hovered);
            }
            else
            {
                if (!ObjectHighlighted)
                    ApplySelectionState(Actor, ESelectionState::SelectedAndHovered);
            }
        }
    }
}


//Set Marquee Widget size and position in Viewport
void USelectionSystemComponent::UpdateMarqueeWidget(FVector2D StartPosition, FVector2D FinishPosition) const
{
	if (!MarqueeWidget) return;
	
	int32 SizeX = 0;
	int32 SizeY = 0;

	APController->GetViewportSize(SizeX, SizeY);

	//Fixing visual frame bug with very large sizes
	StartPosition.X = UKismetMathLibrary::FClamp(StartPosition.X, -100, SizeX + 100);
	StartPosition.Y = UKismetMathLibrary::FClamp(StartPosition.Y, -100, SizeY + 100);

	FinishPosition.X = UKismetMathLibrary::FClamp(FinishPosition.X, -100, SizeX + 100);
	FinishPosition.Y = UKismetMathLibrary::FClamp(FinishPosition.Y, -100, SizeY + 100);

	FVector2D DesiredSize = (StartPosition - FinishPosition);

	if (ViewportScale > 0.01)
	{
		DesiredSize = DesiredSize / ViewportScale;
	}

	MarqueeWidget->SetDesiredSizeInViewport(DesiredSize.GetAbs());
	MarqueeWidget->SetPositionInViewport(StartPosition, true);

	//Set Marquee Widget visible
	if (!MarqueeWidget->IsVisible())
	{
		MarqueeWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}


// ACTORS REGISTRATION
void USelectionSystemComponent::RegisterSelectable(AActor* Actor, FObjectSelectionSettings Settings)
{
    // Check that the actor is valid.
    if (!Actor)
    {
        UE_LOG(LogSelectionManager, Warning, TEXT("Object registration failed: Actor is not valid"));
        return;
    }

    // Check if this actor is already registered.
    for (int i = 0; i < RegisteredActors.Num(); i++)
    {
        if (RegisteredActors[i].Actor == Actor)
        {
            UE_LOG(LogSelectionManager, Warning, TEXT("Object registration failed: Actor %s is already registered"), *Actor->GetName());
            return;
        }
    }

	// Validate that the actor has a SelectableComponent.
	if (!Actor->FindComponentByClass<USelectableComponent>())
	{
		UE_LOG(LogSelectionManager, Warning, TEXT("Object registration failed: Actor %s does not have a SelectableComponent"), *Actor->GetName());
		return;
	}


    // Since only actors are registered, mark the settings accordingly.
    Settings.Actor = Actor;
	
    // (Optional) You can obtain the actor's transform if needed.
    FTransform ObjectTransform = Actor->GetActorTransform();

    // If the object cannot be selected via marquee, simply add the settings and return.
    if (!Settings.CanBeSelectedWithMarquee)
    {
        RegisteredActors.Add(Settings);
        return;
    }

    // Handle dynamic collisions.
    if (Settings.UseDynamicCollisions)
    {
        RegisteredActors.Add(Settings);

        // Debug: Spawn debug collision using only the actor.
        if (bShowDebugCollisions)
            SpawnDebugCollision(Actor, nullptr, Settings.CollisionLODs, true);
        return;
    }

    // Use simple collision if UseSimpleCollision is true or if no collision LODs are defined.
    if (Settings.UseSimpleCollision || Settings.CollisionLODs.Num() <= 0)
    {
        Settings.UseSimpleCollision = true;

        FSelectCollision Collisions;
        FSphereCollision SphereCollision;
        SphereCollision.CenterOffset = FVector::ZeroVector;
        SphereCollision.Radius = 0.0f;
        Collisions.Collisions.Add(SphereCollision);

        Settings.CollisionLODs.Empty();
        Settings.CollisionLODs.Add(Collisions);

        RegisteredActors.Add(Settings);

        if (bShowDebugCollisions)
            SpawnDebugCollision(Actor, nullptr, Settings.CollisionLODs, Settings.UseDynamicCollisions);
        return;
    }

    // Use complex collision.
    for (int i = 0; i < Settings.CollisionLODs.Num(); i++)
    {
        TArray<FSphereCollision>& Collisions = Settings.CollisionLODs[i].Collisions;
        const int32 CollisionsNum = Collisions.Num();

        if (CollisionsNum > 0)
        {
            FSphereCollision Bounds;
            if (CollisionsNum > 1)
            {
                CalculateCollisionBounds(Collisions, Bounds.CenterOffset, Bounds.Radius);
                Settings.LODsBounds.Add(Bounds);
                Settings.NeedBounds.Add(true);
            }
            else
            {
                Settings.LODsBounds.Add(Bounds);
                Settings.NeedBounds.Add(false);
            }

            for (int j = 0; j < CollisionsNum; j++)
            {
                Collisions[j].Radius *= Collisions[j].Radius;
            }

            // Square the switching distance for LOD.
            if (Settings.CollisionLODs[i].SwitchingDistanceLOD > 0.01)
            {
                Settings.CollisionLODs[i].SwitchingDistanceLOD *= Settings.CollisionLODs[i].SwitchingDistanceLOD;
                Settings.LODsSwitchDistances.Add(Settings.CollisionLODs[i].SwitchingDistanceLOD);
            }
            else
            {
                Settings.LODsSwitchDistances.Add(0.0f);
            }
        }
        else
        {
            FSphereCollision SphereCollision;
            Settings.LODsBounds.Add(SphereCollision);
            Settings.NeedBounds.Add(false);

            SphereCollision.CenterOffset = ObjectTransform.GetLocation();
            SphereCollision.Radius = 0.0f;
            Collisions.Add(SphereCollision);

            if (Settings.CollisionLODs[i].SwitchingDistanceLOD > 0.01)
            {
                Settings.CollisionLODs[i].SwitchingDistanceLOD *= Settings.CollisionLODs[i].SwitchingDistanceLOD;
                Settings.LODsSwitchDistances.Add(Settings.CollisionLODs[i].SwitchingDistanceLOD);
            }
            else
            {
                Settings.LODsSwitchDistances.Add(0.0f);
            }
            UE_LOG(LogSelectionManager, Warning, TEXT("Found an empty collision array (LOD# %d) and filled with a simple collision"), i);
        }
    }

    RegisteredActors.Add(Settings);

    // Debug: Spawn collision debug using the actor only.
    if (bShowDebugCollisions)
        SpawnDebugCollision(Actor, nullptr, Settings.CollisionLODs, Settings.UseDynamicCollisions);
}


void USelectionSystemComponent::UnregisterSelectable(AActor* Actor)
{
	if (!Actor)
	{
		UE_LOG(LogSelectionManager, Warning, TEXT("Object unregistration failed: Actor is not valid"));
		return;
	}

	bool ObjectFound = false;
	int32 RemoveIndex = 0;

	// Search RegisteredActors for an entry matching the actor.
	for (int i = 0; i < RegisteredActors.Num(); i++)
	{
		if (RegisteredActors[i].Actor == Actor)
		{
			RemoveIndex = i;
			ObjectFound = true;
			break;
		}
	}

	if (ObjectFound)
	{
		// Retrieve the SelectableComponent directly from the actor.
		USelectableComponent* SelComp = Actor->FindComponentByClass<USelectableComponent>();
		if (SelComp)
		{
			// Optionally update its selection state to Unselected before unregistering,
			// using the helper function so that team-affiliation/ownership checks are applied.
			ApplySelectionState(Actor, ESelectionState::Unselected);
		}
		RegisteredActors.RemoveAt(RemoveIndex);
	}
	else
	{
		UE_LOG(LogSelectionManager, Warning, TEXT("Object unregistration failed: Actor %s was not registered"), *Actor->GetName());
	}
}




//HELPER FUNCTIONS
void USelectionSystemComponent::CalculateCollisionBounds(TArray<struct FSphereCollision>& Collisions, FVector& BoundsOffset, float& BoundsRadius)
{
	if (Collisions.Num() <= 1)
	{
		BoundsOffset.Set(0, 0, 0);
		BoundsRadius = 0.0f;
		return;
	}

	const FVector Offset = Collisions[0].CenterOffset;
	const float Radius = Collisions[0].Radius;

	float MaxX = Offset.X + Radius;
	float MinX = Offset.X - Radius;
	float MaxY = Offset.Y + Radius;
	float MinY = Offset.Y - Radius;
	float MaxZ = Offset.Z + Radius;
	float MinZ = Offset.Z - Radius;

	for (int i = 0; i < Collisions.Num(); i++)
	{
		const FVector OffsetI = Collisions[i].CenterOffset;
		const float RadiusI = Collisions[i].Radius;

		if (OffsetI.X + RadiusI > MaxX) MaxX = OffsetI.X + RadiusI;
		if (OffsetI.X - RadiusI < MinX) MinX = OffsetI.X - RadiusI;

		if (OffsetI.Y + RadiusI > MaxY) MaxY = OffsetI.Y + RadiusI;
		if (OffsetI.Y - RadiusI < MinY) MinY = OffsetI.Y - RadiusI;

		if (OffsetI.Z + RadiusI > MaxZ) MaxZ = OffsetI.Z + RadiusI;
		if (OffsetI.Z - RadiusI < MinZ) MinZ = OffsetI.Z - RadiusI;
	}

	FVector BoxCorner1;
	FVector BoxCorner2;

	BoxCorner1.Set(MaxX, MaxY, MaxZ);
	BoxCorner2.Set(MinX, MinY, MinZ);
	const FVector BoxDiagonal = (BoxCorner1 - BoxCorner2) * 0.5f;

	BoundsOffset = BoxDiagonal + BoxCorner2;
	BoundsRadius = BoxDiagonal.Size();
	BoundsRadius *= BoundsRadius;
}


int32 USelectionSystemComponent::GetCurrentLOD(TArray<float>& Distances, const float SquaringDistanceToObject) const
{
	if (bOrthographicProjection) return 0;

	if (Distances.Num() <= 0) return 0;

	for (int i = 0; i < Distances.Num(); i++)
	{
		if (Distances[i] > SquaringDistanceToObject)
		{
			if (i == 0) return 0;
			return i - 1;
		}
	}

	return Distances.Num() - 1;
}


bool USelectionSystemComponent::ProjectWorldToScreenBidirectional(const FVector& WorldPosition, FVector2D& ScreenPosition) const
{
	FVector Projected;
	bool bSuccess = false;

	const ULocalPlayer* LP = APController ? APController->GetLocalPlayer() : nullptr;
	if (LP && LP->ViewportClient)
	{
		// Get the projection data
		FSceneViewProjectionData ProjectionData;
		if (LP->GetProjectionData(LP->ViewportClient->Viewport, ProjectionData))
		{
			const FMatrix ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
			const FIntRect ViewRectangle = ProjectionData.GetConstrainedViewRect();

			FPlane Result = ViewProjectionMatrix.TransformFVector4(FVector4(WorldPosition, 1.f));
			if (Result.W == 0.f) Result.W = 1.f; // Prevent Divide By Zero

			const float RHW = 1.f / FMath::Abs(Result.W);
			Projected = FVector(Result.X, Result.Y, Result.Z) * RHW;

			// Normalize to 0..1 UI Space
			const float NormX = (Projected.X / 2.f) + 0.5f;
			const float NormY = 1.f - (Projected.Y / 2.f) - 0.5f;

			Projected.X = (float)ViewRectangle.Min.X + (NormX * (float)ViewRectangle.Width());
			Projected.Y = (float)ViewRectangle.Min.Y + (NormY * (float)ViewRectangle.Height());

			bSuccess = true;
			ScreenPosition = FVector2D(Projected.X, Projected.Y);

			if (bPlayerViewportRelative)
				ScreenPosition -= FVector2D(ProjectionData.GetConstrainedViewRect().Min);
		}

		else ScreenPosition = FVector2D(0, 0);
	}

	return bSuccess;
}

bool USelectionSystemComponent::CompareDistWithRadius(const FVector& P, const FVector& PB, const FVector& PN, const float R)
{
	return FVector::DistSquared(UKismetMathLibrary::ProjectPointOnToPlane(P, PB, PN), P) > R;
}


bool USelectionSystemComponent::CheckRightAndLeftPlanes(const FVector& PointDirection, const FVector& PointLocation, float SquareRadius) const
{
	if (FVector::DotProduct(RightPlaneNormal, PointDirection) > 0.0)
	{
		if (FVector::DotProduct(LeftPlaneNormal, PointDirection) > 0.0) return true;
		return !CompareDistWithRadius(PointLocation, CameraLocation, LeftPlaneNormal, SquareRadius);
	}

	return !CompareDistWithRadius(PointLocation, CameraLocation, RightPlaneNormal, SquareRadius);
}


bool USelectionSystemComponent::CheckRightAndLeftPlanesOrt(const FVector& PointLocation, float SquareRadius) const
{
	if (FVector::DotProduct(RightPlaneNormal, PointLocation - RTPoint) > 0.0)
	{
		if (FVector::DotProduct(LeftPlaneNormal, PointLocation - LTPoint) > 0.0) return true;
		return !CompareDistWithRadius(PointLocation, LTPoint, LeftPlaneNormal, SquareRadius);
	}

	return !CompareDistWithRadius(PointLocation, RBPoint, RightPlaneNormal, SquareRadius);
}

bool USelectionSystemComponent::IsSelected(ESelectionState State)
{
	return State == ESelectionState:: SelectedAndHovered || State == ESelectionState::Selected; // return State == ESelectionState::E_SH || State == ESelectionState::E_SuH;
}

bool USelectionSystemComponent::IsHighlighted(ESelectionState State)
{
	return State == ESelectionState::SelectedAndHovered || State == ESelectionState::Hovered; // return State == ESelectionState::E_SH || State == ESelectionState::E_uSH;
}

void USelectionSystemComponent::RemoveNullObjects()
{
	TArray<int32> RemoveIndexes;

	for (int i = 0; i < RegisteredActors.Num(); i++)
	{
		if (IsValid(RegisteredActors[i].Actor)) continue;

		RemoveIndexes.Add(i);
	}

	Algo::Reverse(RemoveIndexes);

	for (int i = 0; i < RemoveIndexes.Num(); i++)
	{
		RegisteredActors.RemoveAt(RemoveIndexes[i]);
	}
}


//DEBUG
void USelectionSystemComponent::SpawnDebugCollision(
	AActor* Actor, 
	USceneComponent* Component, 
	const TArray<struct FSelectCollision>& Collisions, 
	const bool UseDynamicCollisions)
{
	if (!DebuggerClass)
	{
		UE_LOG(LogSelectionManager, Warning, TEXT("DebuggerClass is not set"));
		return;
	}

	FVector SpawnLocation;
	FRotator SpawnRotation;

	if (Component)
	{
		SpawnLocation = Component->GetComponentLocation();
		SpawnRotation = Component->GetComponentRotation();
	}

	if (Actor)
	{
		SpawnLocation = Actor->GetActorLocation();
		SpawnRotation = Actor->GetActorRotation();
	}

	ASelectionSystemDebugger* Debugger = GetWorld()->SpawnActor<ASelectionSystemDebugger>(DebuggerClass, SpawnLocation, SpawnRotation);
	Debugger->SetupDebugger(ACameraManager, this, Actor, Collisions, UseDynamicCollisions);
}

void USelectionSystemComponent::ApplySelectionState(AActor* Actor, ESelectionState DesiredState) const
{
	if (!Actor || !GetOwner()) return;

	if (APlayerController* LocalPC = Cast<APlayerController>(GetOwner()))
	{
		// Use FindComponentByClass for SelectableComponent
		USelectableComponent* Selectable = Actor->FindComponentByClass<USelectableComponent>();

		if (Selectable)
		{
			if (Selectable->OwnershipType == EOwnershipType::Neutral)
			{
				Selectable->UpdateActorSelection(DesiredState, ETeamAffiliation::Neutral);
				return;
			}

			if (!PlayerControllerTeamComponent)
			{
				CachePlayerControllerTeam(LocalPC);
			}

			// Use FindComponentByClass for TeamComponent
			UTeamComponent* ActorTeam = Actor->FindComponentByClass<UTeamComponent>();

			const ETeamAffiliation Affiliation = CalculateTeamAffiliation(ActorTeam);
			Selectable->UpdateActorSelection(DesiredState, Affiliation);
		}
	}
}


ETeamAffiliation USelectionSystemComponent::CalculateTeamAffiliation(UTeamComponent* ActorTeam) const
{
    if (!ActorTeam || !PlayerControllerTeamComponent)
    {
        return ETeamAffiliation::Neutral;
    }

    if (ActorTeam->GetPlayerOwner() == PlayerControllerTeamComponent->GetPlayerOwner())
    {
        return ETeamAffiliation::Owned;
    }
    
    return (ActorTeam->GetTeamIndex() == PlayerControllerTeamComponent->GetTeamIndex())
        ? ETeamAffiliation::Ally
        : ETeamAffiliation::Enemy;
}

void USelectionSystemComponent::CachePlayerControllerTeam(APlayerController* LocalPlayerController) const
{
    if (LocalPlayerController)
    {
        PlayerControllerTeamComponent = LocalPlayerController->FindComponentByClass<UTeamComponent>();
    }
}




