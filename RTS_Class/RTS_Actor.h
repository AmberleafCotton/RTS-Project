// Copyright AmberleafCotton 2025. All Rights Reserved.
#pragma once

#include "GameplayTagContainer.h"
#include "GameFramework/Pawn.h"
#include "RTS_Actor.generated.h"

class URTS_DataAsset;
class URTS_Module;

UCLASS()
class FINALRTS_API ARTS_Actor : public APawn
{
	GENERATED_BODY()

public:
	ARTS_Actor();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RTS Actor")
	void Initialize();
	virtual void Initialize_Implementation();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Actor", meta = (ExposeOnSpawn = true))
	URTS_DataAsset* ActorDataAsset = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "RTS Actor")
	TMap<FGameplayTag, TObjectPtr<URTS_Module>> Modules;

	UFUNCTION(BlueprintPure, Category = "RTS Actor")
	FGameplayTag GetGameplayTag() const;

	UFUNCTION(BlueprintPure, Category = "RTS Actor")
	FGameplayTagContainer GetRTS_Tags() const;
};
