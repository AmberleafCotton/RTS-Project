// Copyright AmberleafCotton 2025. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "RTS_Module.h"
#include "BasicData.h"
#include "MeshData.h"
#include "GameplayTagContainer.h"
#include "InputMappingContext.h"
#include "RTS_DataAsset.generated.h"

/**
 * URTS_DataAsset is a data asset class used to hold core data for RTS modules and associated settings.
 * It contains an array of instanced RTS modules, input mapping context, and gameplay tags for classification.
 * This class allows for easy configuration and customization of modules for RTS units, buildings, or other gameplay elements.
 * 
 * The data asset is intended to be used for setting up various properties and configurations 
 * for RTS modules within the game, including basic data, tags, and input mappings.
 */
UCLASS(BlueprintType)
class FINALRTS_API URTS_DataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Basic data for the RTS entity associated with this data asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Actor")
	FBasicData BasicData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "RTS Actor")
	TMap<FGameplayTag, TObjectPtr<URTS_Module>> Modules;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Actor")
	FMeshData MeshData;
	
	/** A gameplay tag representing the gameplay type or role this entity holds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Actor")
	FGameplayTag GameplayTag = {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Actor")
	FGameplayTagContainer RTS_Tags;

	/** Additional tags for custom classification or grouping of this data asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Actor")
	TArray<FName> Tags = {};
	
	/** Input context to be used when the entity is hovered (e.g., for UI input mappings). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Actor")
	UInputMappingContext* HoveredContext;

	/** Input context to be used when the entity is hovered (e.g., for UI input mappings). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Actor")
	UInputMappingContext* SelectedContext;
	
	/** A gameplay tag to identify the base type of the entity this data asset is associated with. */
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Actor")
	// FGameplayTag BaseGameplayTag = {};
	//
	// /** A gameplay tag to signify that this entity is interactable within the gameplay context. */
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Actor")
	// FGameplayTag GameplayInteractableTag = {};
};
