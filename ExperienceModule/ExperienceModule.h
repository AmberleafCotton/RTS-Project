#pragma once

#include "CoreMinimal.h"
#include "RTS_Module.h"
#include "ExperienceModule.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUp, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExperienceGained, int32, XPAdded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExperienceUpdate, int32, CurrentXP, int32, RequiredXP);

UCLASS(Abstract, Blueprintable, EditInlineNew)
class FINALRTS_API UExperienceModule : public URTS_Module
{
	GENERATED_BODY()

public:
	UExperienceModule();

	virtual void InitializeModule_Implementation(ARTS_Actor* InOwner) override;

	UPROPERTY(BlueprintReadOnly, Category = "Experience Module")
	int32 CurrentLevel = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Experience Module")
	int32 CurrentXP = 0;

	// Settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Experience Module")
	int32 MaxLevel = 40;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Experience Module")
	TMap<int32, int32> XPRequirements;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Experience Module")
	FOnLevelUp OnLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "Experience Module")
	FOnExperienceGained OnExperienceGained;

	UPROPERTY(BlueprintAssignable, Category = "Experience Module")
	FOnExperienceUpdate OnExperienceUpdate;

	// API
	UFUNCTION(BlueprintCallable, Category = "Experience Module")
	void AddExperience(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Experience Module")
	int32 GetCurrentLevel() const;

	UFUNCTION(BlueprintPure, Category = "Experience Module")
	int32 GetXPToNextLevel() const;

private:
	// Internal logic
	void LevelUp();
}; 