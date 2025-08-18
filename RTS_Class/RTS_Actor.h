// Copyright AmberleafCotton 2025. All Rights Reserved.
#pragma once

#include "GameFramework/Character.h"
#include "RTS_ActorInterface.h"
#include "GameplayTagContainer.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"

#include "RTS_Actor.generated.h"

class UInputMappingContext;
class URTS_DataAsset;
class URTS_Module;

UCLASS()
class DRAKTHYSPROJECT_API ARTS_Actor : public ACharacter, public IRTS_ActorInterface
{
	GENERATED_BODY()

public:
	ARTS_Actor();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RTS Actor")
	void Initialize();
	virtual void Initialize_Implementation();

	UFUNCTION(Category = "RTS Actor")
	void InitializeModules();

	// Component setup functions
	UFUNCTION(BlueprintCallable, Category = "RTS Actor")
	void InitializeSelectedWidget();
	
	UFUNCTION(BlueprintCallable, Category = "RTS Actor")
	void SetupActorComponents();
	
	UFUNCTION(BlueprintCallable, Category = "RTS Actor")
	void SetupAsCharacter();
	
	UFUNCTION(BlueprintCallable, Category = "RTS Actor")
	void SetupAsStaticBuilding();

	// Initialize Mesh, Navigation Box, 
	UFUNCTION(BlueprintCallable, Category = "RTS Actor")
	void InitializeMesh();
	
	// Initialize Navigation Box separately
	UFUNCTION(BlueprintCallable, Category = "RTS Actor")
	void InitializeNavigationBox();
	
	// Initialize Placement Box for building placement
	UFUNCTION(BlueprintCallable, Category = "RTS Actor")
	void InitializePlacementBox();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RTS Actor")
	TObjectPtr<USceneComponent> RTS_Actor = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RTS Actor")
	TObjectPtr<UStaticMeshComponent> RTS_StaticMesh = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RTS Actor")
	TObjectPtr<UBillboardComponent> RTS_Billboard = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS Actor")
	TObjectPtr<UBoxComponent> RTS_NavigationBox = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RTS Actor")
	TObjectPtr<UBoxComponent> RTS_PlacementBox = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Actor", meta = (ExposeOnSpawn = true))
	TObjectPtr<URTS_DataAsset> ActorDataAsset = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "RTS Actor")
	TMap<FGameplayTag, TObjectPtr<URTS_Module>> Modules;

	// Access to RTS_DataAsset variables
	UFUNCTION(BlueprintCallable, Category = "RTS Input")
	UInputMappingContext* GetSelectedContext() const;

	UFUNCTION(BlueprintCallable, Category = "RTS Input")
	UInputMappingContext* GetHoveredContext() const;

	UFUNCTION(BlueprintPure, Category = "RTS Actor")
	FGameplayTag GetRTSGameplayTag() const;
	
	UFUNCTION(BlueprintPure, Category = "RTS Actor")
	FName GetRTS_ActorTypeTag() const;

	UFUNCTION(BlueprintPure, Category = "RTS Actor")
	FGameplayTagContainer GetRTS_Tags() const;
	
	// Selected Widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Widget")
	TObjectPtr<UUserWidget> SelectedWidget;
	
	UFUNCTION(BlueprintCallable, Category = "RTS Widget")
	UUserWidget* GetSelectedWidget() const;
};
