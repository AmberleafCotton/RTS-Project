#include "RTS_Actor.h"
#include "RTS_DataAsset.h"
#include "RTS_Module.h"
#include "AI/NavigationSystemBase.h"
#include "WidgetComponent/WidgetsComponent.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavAreas/NavArea_Obstacle.h"

ARTS_Actor::ARTS_Actor()
{
	// Create RTS_Actor SceneComponent
	RTS_Actor = CreateDefaultSubobject<USceneComponent>(TEXT("RTS_Actor"));
	RTS_Actor->SetupAttachment(RootComponent);

	// Create RTS_Actor StaticMeshComponent
	RTS_StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RTS_StaticMesh"));
	RTS_StaticMesh->SetupAttachment(RTS_Actor);
	// Settings for StaticMesh
	RTS_StaticMesh->PrimaryComponentTick.bCanEverTick = false;
	RTS_StaticMesh->SetComponentTickEnabled(false);
	RTS_StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RTS_StaticMesh->SetGenerateOverlapEvents(false);
	RTS_StaticMesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

	// Create RTS_Actor BillboardComponent
	RTS_Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("RTS_Billboard"));
	RTS_Billboard->SetupAttachment(RTS_Actor);
	// Settings for Billboard
	RTS_Billboard->SetHiddenInGame(false);
	RTS_Billboard->SetVisibility(true);

	// Create RTS_Navigation Box Collision 
	RTS_NavigationBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RTS_NavigationBox"));
	RTS_NavigationBox->SetupAttachment(RTS_Actor);
	// Settings for Navigation Effect 
	RTS_NavigationBox->InitBoxExtent(FVector(50.f, 50.f, 50.f));
	RTS_NavigationBox->ShapeColor = FColor(255, 180, 0);
	RTS_NavigationBox->SetLineThickness(5.f);
	RTS_NavigationBox->SetCanEverAffectNavigation(true);
	RTS_NavigationBox->SetGenerateOverlapEvents(false);
	RTS_NavigationBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RTS_NavigationBox->SetCollisionProfileName(TEXT("Navigation"));
	RTS_NavigationBox->SetHiddenInGame(false);
	RTS_NavigationBox->SetVisibility(true);
	RTS_NavigationBox->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	RTS_NavigationBox->SetComponentTickEnabled(false);
	RTS_NavigationBox->bDynamicObstacle = true;
	RTS_NavigationBox->SetAreaClassOverride(UNavArea_Obstacle::StaticClass());

	// Create RTS_Placement Box Collision for building placement
	RTS_PlacementBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RTS_PlacementBox"));
	RTS_PlacementBox->SetupAttachment(RTS_Actor);
	// Settings for Placement Effect 
	RTS_PlacementBox->InitBoxExtent(FVector(100.f, 100.f, 50.f));
	RTS_PlacementBox->ShapeColor = FColor(0, 255, 0);
	RTS_PlacementBox->SetLineThickness(3.f);
	RTS_PlacementBox->SetCanEverAffectNavigation(false);
	RTS_PlacementBox->SetGenerateOverlapEvents(true);
	RTS_PlacementBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RTS_PlacementBox->SetCollisionProfileName(TEXT("Placement"));
	RTS_PlacementBox->SetHiddenInGame(false);
	RTS_PlacementBox->SetVisibility(true);
	RTS_PlacementBox->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	RTS_PlacementBox->SetComponentTickEnabled(false);

	// Capsule Component Visuals
	GetCapsuleComponent()->SetLineThickness(2.f);
}

void ARTS_Actor::Initialize_Implementation()
{
	// 1. Initialize modules from data asset (if available)
	if (ActorDataAsset)
	{
		InitializeModules();
	}

	// 2. Setup widget components for UI feedback
	InitializeSelectedWidget();
	
	// 3. Setup actor components based on movement capability
	//    - Characters (with Movement module): Keep character components, remove static components
	//    - Buildings (no Movement module): Keep static components, remove character components
	SetupActorComponents();
}

void ARTS_Actor::InitializeModules()
{
	// Process each module from the data asset
	for (const TPair<FGameplayTag, TObjectPtr<URTS_Module>>& Pair : ActorDataAsset->Modules)
	{
		const FGameplayTag& Tag = Pair.Key;
		const TObjectPtr<URTS_Module>& Module = Pair.Value;

		// Skip invalid modules
		if (!Module)
		{
			continue;
		}

		// Create a copy of the module for this actor
		URTS_Module* DuplicatedModule = DuplicateObject<URTS_Module>(Module, this);
		if (!DuplicatedModule)
		{
			continue;
		}

		// Initialize the module with this actor as owner
		DuplicatedModule->InitializeModule(this);

		// Add the module to the appropriate category in this actor
		Modules.Add(Tag, DuplicatedModule);
	}
}

void ARTS_Actor::InitializeSelectedWidget()
{
	// Setup widget components for UI feedback
	if (TObjectPtr<UWidgetsComponent> WidgetComponent = FindComponentByClass<UWidgetsComponent>())
	{
		// Get the widget from the component, but only if it's valid
		UUserWidget* WidgetFromComponent = WidgetComponent->GetSelectedWidget();
		if (WidgetFromComponent)
		{
			SelectedWidget = WidgetFromComponent;
			UE_LOG(LogTemp, Log, TEXT("SelectedWidget initialized from WidgetsComponent for %s"), *GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("WidgetsComponent found but no SelectedWidget available for %s"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("No WidgetsComponent found for %s - SelectedWidget will remain null"), *GetName());
	}
}

void ARTS_Actor::SetupActorComponents()
{
	// Determine if this actor should be a character (movable) or static building
	bool bShouldBeCharacter = false;
	
	if (ActorDataAsset)
	{
		// Check if actor has movement module - if yes, it should be a character
		bShouldBeCharacter = ActorDataAsset->Modules.Contains(FGameplayTag::RequestGameplayTag("Module.Movement"));
	}
	
	// Setup components based on actor type
	if (bShouldBeCharacter)
	{
		SetupAsCharacter();
	}
	else
	{
		SetupAsStaticBuilding();
	}
}

void ARTS_Actor::SetupAsCharacter()
{
	// Character actors need movement components, remove static building components
	
	// Remove navigation box for movable actors (they don't need static navigation)
	if (RTS_NavigationBox)
	{
		RTS_NavigationBox->DestroyComponent();
	}

	if (RTS_PlacementBox)
	{
		RTS_PlacementBox->DestroyComponent();
	}
	
	// Initialize character-specific components
	InitializeMesh();
}

void ARTS_Actor::SetupAsStaticBuilding()
{
	// Static buildings need static components, remove character components
	
	// Remove character movement components
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DestroyComponent();
	}

	// Remove character mesh
	if (GetMesh())
	{
		GetMesh()->DestroyComponent();
	}
	
	// Remove arrow component if present
	if (UArrowComponent* Arrow = FindComponentByClass<UArrowComponent>())
	{
		Arrow->DestroyComponent();
	}
	
	// Initialize static building components
	InitializeMesh();
	InitializeNavigationBox();
	InitializePlacementBox();
}

// If the RTS Actor is not Character then Initialize Mesh and Navigation Box
void ARTS_Actor::InitializeMesh()
{
	if (!ActorDataAsset)
		return;

	// Might need to switch on Skeletal Mesh
	if (ActorDataAsset->MeshData.StaticMesh && RTS_StaticMesh)
	{
		RTS_StaticMesh->SetStaticMesh(const_cast<UStaticMesh*>(ActorDataAsset->MeshData.StaticMesh));
		RTS_StaticMesh->SetMaterial(0, ActorDataAsset->MeshData.Material);
	}
}

void ARTS_Actor::InitializeNavigationBox()
{
	if (!ActorDataAsset || !RTS_NavigationBox)
		return;

	// Navigation Box Sizing
	FVector NavigationExtent = ActorDataAsset->MeshData.NavigationExtent;
	RTS_NavigationBox->SetBoxExtent(NavigationExtent);
	RTS_NavigationBox->UpdateBounds();
	FNavigationSystem::UpdateComponentData(*RTS_NavigationBox);
}

void ARTS_Actor::InitializePlacementBox()
{
	if (!ActorDataAsset || !RTS_PlacementBox)
		return;

	// Calculate placement box extent based on tile count
	// GetDistanceFromTileCount(TileCount) / 2 + 100,100,0
	float TileSize = 100.0f;
	FVector2D TileCount = ActorDataAsset->MeshData.TileCount;
	
	// Calculate base size from tile count
	float BaseX = TileCount.X * TileSize;
	float BaseY = TileCount.Y * TileSize;
	
	// Add 100 units to each side (1 tiledfdfd than building)
	float PlacementX = (BaseX / 2.0f) + 80.0f;	
	float PlacementY = (BaseY / 2.0f) + 80.0f;
	
	FVector PlacementExtent = FVector(PlacementX, PlacementY, 25.0f);
	RTS_PlacementBox->SetBoxExtent(PlacementExtent);
	RTS_PlacementBox->UpdateBounds();
	RTS_PlacementBox->ComponentTags.AddUnique(FName("PlacementBox"));
}

UInputMappingContext* ARTS_Actor::GetSelectedContext() const
{
	return ActorDataAsset ? ActorDataAsset->SelectedContext : nullptr;
}

UInputMappingContext* ARTS_Actor::GetHoveredContext() const
{
	return ActorDataAsset ? ActorDataAsset->HoveredContext : nullptr;
}

FGameplayTag ARTS_Actor::GetRTSGameplayTag() const
{
	return ActorDataAsset->RTS_GameplayTag;
}

FName ARTS_Actor::GetRTS_ActorTypeTag() const
{
	return ActorDataAsset->RTS_ActorTypeTag;
}

FGameplayTagContainer ARTS_Actor::GetRTS_Tags() const
{
	return ActorDataAsset->RTS_Tags;
}

UUserWidget* ARTS_Actor::GetSelectedWidget() const
{
	return SelectedWidget;
}
