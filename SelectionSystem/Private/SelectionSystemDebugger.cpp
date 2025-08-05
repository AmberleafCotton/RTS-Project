// Copyright 2025 AmberleafCotton. All right reserved.

#include "SelectionSystemDebugger.h"
#include "MeshDescription.h"
#include "SelectableComponent.h"
#include "SelectionSystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"

// SETS DEFAULT VALUES
ASelectionSystemDebugger::ASelectionSystemDebugger()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DebuggerRoot"));
	ActiveLOD = - 1;
}


// BEGIN PLAY
void ASelectionSystemDebugger::BeginPlay()
{
	Super::BeginPlay();

}

void ASelectionSystemDebugger::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// EDITOR MODE
	if (bEditorMode)
	{
		SpawnCollisions(PreviewSelectionCollisions);

		if (ShowLODNumber < 0) ShowLODNumber = 0;

		if (ShowLODNumber > CollisionLODs.Num() - 1) ShowLODNumber = CollisionLODs.Num() - 1;

		ShowLODByIndex(ShowLODNumber);
	}
}

// TICK
void ASelectionSystemDebugger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Since we now register only actors, we only check the validity of ActorOwner.
    if (!IsValid(ActorOwner))
    {
        Destroy();
        return;
    }

    // In Editor Mode:
    // If dynamic collisions are used, we do nothing in Tick.
    // Otherwise, we switch LODs based on distance.
    if (bEditorMode)
    {
        if (bUseDynamicCollisions)
            return;

        SwitchLODsEditorMode();
        return;
    }

    // In Debug Mode, if the debugger isn't ready, do nothing.
    if (!bDebuggerReady)
        return;

    // Set NewLOD to the highest possible LOD index initially.
    NewLOD = DistancesLODsSwitch.Num() - 1;

    // Get the camera's current location.
    const FVector CameraLocation = CameraManager->GetCameraLocation();
    // Since we register only actors, get the location and rotation from ActorOwner.
    FVector ObjectLocation = ActorOwner->GetActorLocation();
    FRotator ObjectRotation = ActorOwner->GetActorRotation();

    // Compute a rough "squared distance" from the camera to the object.
    // (This avoids using the full vector length for efficiency.)
    FVector ObjectDirection = CameraLocation - ObjectLocation;
    ObjectDirection *= ObjectDirection; // element-wise multiplication
    const float SquareDistance = ObjectDirection.X + ObjectDirection.Y + ObjectDirection.Z;

    // Update the debugger actor's own transform to follow the ActorOwner.
    SetActorLocation(ObjectLocation);
    SetActorRotation(ObjectRotation);

    // If using dynamic collisions, update the dynamic collision LOD.
    if (bUseDynamicCollisions)
        MoveDynamicCollisionLOD(ActiveLOD);

    // Determine which LOD should be active based on the squared distance.
    for (int i = 0; i < DistancesLODsSwitch.Num(); ++i)
    {
        if (DistancesLODsSwitch[i] > SquareDistance)
        {
            NewLOD = (i == 0) ? 0 : i - 1;
            break;
        }
    }

    // If the LOD hasn't changed, no need to update.
    if (NewLOD == ActiveLOD)
        return;

    // Update the active LOD and show the corresponding collision visualization.
    ActiveLOD = NewLOD;
    ShowLODByIndex(NewLOD);
}



void ASelectionSystemDebugger::SetupDebugger( APlayerCameraManager* PlayerCameraManager,
                                               USelectionSystemComponent* SelectionManagerComponent,
                                               AActor* Actor,
                                               const TArray<FSelectCollision>& Collisions,
                                               bool UseDynamicCollisions)
{
    // Basic sanity checks.
    if (!GetWorld() || !SelectionManagerComponent)
        return;

    if (!RootComponent)
        return;

    // If there are no collisions and we're not using dynamic collisions, nothing to set up.
    if (Collisions.Num() <= 0 && !UseDynamicCollisions)
        return;

    // Since only actors are now registered, require a valid actor.
    if (!Actor)
    {
        UE_LOG(LogSelectionManager, Warning, TEXT("SetupDebugger failed: No valid Actor provided."));
        return;
    }

    bDebuggerReady = true;

    // Retrieve the camera manager from the first player controller.
    CameraManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
    SelectionManager = SelectionManagerComponent;
    SelectionCollisions = Collisions;
    bUseDynamicCollisions = UseDynamicCollisions;

    // Set the ActorOwner and update relative scale.
    ActorOwner = Actor;
    this->SetActorRelativeScale3D(Actor->GetActorScale3D());

    // Use dynamic collisions if enabled.
    if (bUseDynamicCollisions)
    {
        // Retrieve dynamic collisions from the actor's SelectableComponent.
        USelectableComponent* SelComp = Actor->FindComponentByClass<USelectableComponent>();
        if (SelComp)
        {
            DynamicCollisions.Add(SelComp->GetObjectDynamicCollisions());
        }
        else
        {
            UE_LOG(LogSelectionManager, Warning, TEXT("SetupDebugger: Actor %s lacks a SelectableComponent."), *Actor->GetName());
        }
        SpawnCollisions(DynamicCollisions);
    }
    else
    {
        // Otherwise, use the provided collision settings.
        SpawnCollisions(SelectionCollisions);
    }
}



void ASelectionSystemDebugger::SpawnCollisions(TArray<struct FSelectCollision>& Collisions)
{
	CollisionMeshes.Empty();
	BoundsMeshes.Empty();
	CollisionLODs.Empty();
	DistancesLODsSwitch.Empty();
	SelectionCollisionLODsBounds.Empty();
	SimpleBounds.Empty();

	//Create Sphere or Dot Collisions
	for (int i = 0; i < Collisions.Num(); i++)
	{
		int32 Index = -1;
		DistancesLODsSwitch.Add(Collisions[i].SwitchingDistanceLOD);

		if (!bUseDynamicCollisions) CalculateBounds(Collisions[i].Collisions);

		for (int j = 0; j < Collisions[i].Collisions.Num(); j++)
		{
			Index = j;
			const FSphereCollision SphereCollision = Collisions[i].Collisions[j];

			float Radius = SphereCollision.Radius;
			if (!bEditorMode) Radius = sqrt(Radius);

			FVector Scale;
			Scale.Set(Radius, Radius, Radius);

			UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass());

			if (!Mesh) return;

			Mesh->RegisterComponent();
			Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			//Mesh->CreationMethod = EComponentCreationMethod::Instance;
			Mesh->CreationMethod = EComponentCreationMethod::SimpleConstructionScript;

			if (DebugMesh) Mesh->SetStaticMesh(DebugMesh);

			Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Mesh->SetMobility(EComponentMobility::Movable);
			Mesh->SetGenerateOverlapEvents(false);
			Mesh->SetCastShadow(false);
			Mesh->SetCanEverAffectNavigation(false);
			Mesh->SetVisibility(false);

			Mesh->SetRelativeLocation(SphereCollision.CenterOffset);


			//Set Sphere Collision
			if(Radius > 0.01f)
			{
				if (DebugMeshMaterial) Mesh->SetMaterial(0, DebugMeshMaterial);
				Mesh->SetRelativeScale3D(Scale);
				CollisionMeshes.Add(Mesh);
			}

			//Set Dot Collision
			else 
			{
				Mesh->SetAbsolute(false, false, true);
				Scale.Set(8.0f, 8.0f, 8.0f);
				if (DebugDotMaterial) Mesh->SetMaterial(0, DebugDotMaterial);
				Mesh->SetRelativeScale3D(Scale);
				CollisionMeshes.Add(Mesh);
			}
		}

		if (Index >= 0)
		{
			CollisionLODs.Add(CollisionMeshes.Num() - 1);
		}
	}

	//Create Bounds Collision
	for (int i = 0; i < SelectionCollisionLODsBounds.Num(); i++)
	{
		if (SimpleBounds[i]) BoundsMeshes.Add(nullptr);
		else
		{
			FVector Scale;
			const float Radius = SelectionCollisionLODsBounds[i].Radius;
			Scale.Set(Radius, Radius, Radius);

			UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass());

			if (!Mesh) return;
			
			Mesh->RegisterComponent();
			Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			//Mesh->CreationMethod = EComponentCreationMethod::Instance;
			Mesh->CreationMethod = EComponentCreationMethod::SimpleConstructionScript;

			if (DebugMesh) Mesh->SetStaticMesh(DebugMesh);

			Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Mesh->SetMobility(EComponentMobility::Movable);
			Mesh->SetGenerateOverlapEvents(false);
			Mesh->SetCastShadow(false);
			Mesh->SetCanEverAffectNavigation(false);
			Mesh->SetVisibility(false);

			Mesh->SetRelativeLocation(SelectionCollisionLODsBounds[i].CenterOffset);
			Mesh->SetRelativeScale3D(Scale);
			if (DebugBoundsMaterial) Mesh->SetMaterial(0, DebugBoundsMaterial);

			BoundsMeshes.Add(Mesh);
		}
	}
}


void ASelectionSystemDebugger::CalculateBounds(TArray<FSphereCollision>& Collisions)
{
	if (Collisions.Num() <= 1) {
		if(Collisions.Num() > 0) {

			FSphereCollision SphereCollision;
			SphereCollision.CenterOffset.Set(0.0f, 0.0f, 0.0f);
			SphereCollision.Radius = 0.0f;
			SelectionCollisionLODsBounds.Add(SphereCollision);
			SimpleBounds.Add(true);
			return;
		}
		return;
	}


	FVector Offset;

	Offset = Collisions[0].CenterOffset;

	float Radius;
	if (bEditorMode)	Radius = Collisions[0].Radius;
	else				Radius = sqrt(Collisions[0].Radius);

	float MaxX = Offset.X + Radius;
	float MinX = Offset.X - Radius;
	float MaxY = Offset.Y + Radius;
	float MinY = Offset.Y - Radius;
	float MaxZ = Offset.Z + Radius;
	float MinZ = Offset.Z - Radius;

	for (int i = 0; i < Collisions.Num(); i++)
	{
		FVector OffsetI;

		OffsetI = Collisions[i].CenterOffset;

		float RadiusI;

		if (bEditorMode)	RadiusI = Collisions[i].Radius;
		else				RadiusI = sqrt(Collisions[i].Radius);

		if (OffsetI.X + RadiusI > MaxX) MaxX = OffsetI.X + RadiusI;
		if (OffsetI.X - RadiusI < MinX) MinX = OffsetI.X - RadiusI;

		if (OffsetI.Y + RadiusI > MaxY) MaxY = OffsetI.Y + RadiusI;
		if (OffsetI.Y - RadiusI < MinY) MinY = OffsetI.Y - RadiusI;

		if (OffsetI.Z + RadiusI > MaxZ) MaxZ = OffsetI.Z + RadiusI;
		if (OffsetI.Z - RadiusI < MinZ) MinZ = OffsetI.Z - RadiusI;
	}

	FSphereCollision SphereCollision;
	FVector BoxCorner1;
	FVector BoxCorner2;

	BoxCorner1.Set(MaxX, MaxY, MaxZ);
	BoxCorner2.Set(MinX, MinY, MinZ);

	const FVector BoxDiagonal = (BoxCorner1 - BoxCorner2) * 0.5f;
	const float BoundsRadius = BoxDiagonal.Size();

	SphereCollision.CenterOffset = BoxDiagonal + BoxCorner2;
	SphereCollision.Radius = BoundsRadius;

	SelectionCollisionLODsBounds.Add(SphereCollision);
	SimpleBounds.Add(false);
}


void ASelectionSystemDebugger::ShowLODByIndex(int32 Index)
{
	if (CollisionLODs.Num() <= 0) return;

	for (int i = 0; i < CollisionMeshes.Num(); i++)
		CollisionMeshes[i]->SetVisibility(false);

	if (bShowLODsBounds && !bUseDynamicCollisions)
	{
		for (int i = 0; i < BoundsMeshes.Num(); i++)
			if (BoundsMeshes[i]) BoundsMeshes[i]->SetVisibility(false);
	}


	int32 ShowLOD = Index;

	if (Index < 0) ShowLOD = 0;

	const int32 MaxIndex = CollisionLODs.Num() - 1;

	if (Index > MaxIndex) ShowLOD = MaxIndex;

	int32 StartIndex;
	int32 ArrayIndex;

	if (ShowLOD == 0)	ArrayIndex = 0;
	else				ArrayIndex = ShowLOD - 1;

	if (ShowLOD == 0)	StartIndex = 0;
	else				StartIndex = CollisionLODs[ArrayIndex] + 1;

	for (int i = StartIndex; i <= CollisionLODs[ShowLOD]; i++)
		CollisionMeshes[i]->SetVisibility(true);

	if (BoundsMeshes.Num() <= 0) return;

	if(!SimpleBounds[ShowLOD] && bShowLODsBounds && !bUseDynamicCollisions) 
		BoundsMeshes[ShowLOD]->SetVisibility(true);
}



void ASelectionSystemDebugger::SwitchLODsEditorMode()
{
	const UWorld* World = GetWorld();

	if (!World) return;

	FVector CameraLocation;

	if (World->WorldType == EWorldType::PIE)
	{
		CameraLocation = World->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation();
	}

	const float DistanceToCamera = (CameraLocation - GetActorLocation()).Size();

	NewLOD = DistancesLODsSwitch.Num() - 1;

	for (int i = 0; i < DistancesLODsSwitch.Num(); ++i)
	{
		if (DistancesLODsSwitch[i] > DistanceToCamera)
		{
			if (i == 0)	NewLOD = 0;
			else		NewLOD = i - 1;

			break;
		}
	}

	if (NewLOD == ActiveLOD) return;

	ActiveLOD = NewLOD;
	ShowLODByIndex(NewLOD);
}

void ASelectionSystemDebugger::MoveDynamicCollisionLOD(int32 Index)
{
	if (CollisionLODs.Num() <= 0)
		return;

	// Determine the LOD to show.
	int32 ShowLOD = Index;
	if (Index < 0)
		ShowLOD = 0;
	const int32 MaxIndex = CollisionLODs.Num() - 1;
	if (Index > MaxIndex)
		ShowLOD = MaxIndex;

	int32 StartIndex;
	int32 ArrayIndex;
	if (ShowLOD == 0)
	{
		ArrayIndex = 0;
		StartIndex = 0;
	}
	else
	{
		ArrayIndex = ShowLOD - 1;
		StartIndex = CollisionLODs[ArrayIndex] + 1;
	}

	// Since we now register only actors, directly use ActorOwner.
	AActor* Actor = ActorOwner;
	// (We assume ActorOwner is valid; otherwise, you could check for null.)
	if (!Actor)
		return;

	// Retrieve dynamic collisions directly from the actor's SelectableComponent.
	USelectableComponent* SelComp = Actor->FindComponentByClass<USelectableComponent>();
	if (!SelComp)
		return;
	FSelectCollision DynamicCollision = SelComp->GetObjectDynamicCollisions();

	int32 j = 0;
	// Note: The original loop used CollisionLODs[0]; if that is core logic, we leave it unchanged.
	for (int i = StartIndex; i <= CollisionLODs[0]; i++)
	{
		if (CollisionMeshes.IsValidIndex(i) && DynamicCollision.Collisions.IsValidIndex(j))
		{
			CollisionMeshes[i]->SetWorldLocation(DynamicCollision.Collisions[j].CenterOffset);
		}
		j++;
	}
}

