#include "ExperienceModule.h"

UExperienceModule::UExperienceModule()
{
	// Initialize runtime state
	CurrentXP = 0;
	CurrentLevel = 1;

	// Default XP requirements per level
	XPRequirements = {
		{1, 10}, {2, 14}, {3, 20}, {4, 25}, {5, 30}, {6, 35}, {7, 34}, {8, 39}, {9, 49}, {10, 52},
		{11, 58}, {12, 64}, {13, 69}, {14, 75}, {15, 85}, {16, 120}, {17, 150}, {18, 155}, {19, 169}, {20, 174},
		{21, 195}, {22, 240}, {23, 280}, {24, 420}, {25, 500}, {26, 480}, {27, 460}, {28, 440}, {29, 420}, {30, 400}
	};

	// Ensure MaxLevel matches the XPRequirements map size
	MaxLevel = XPRequirements.Num();
}

void UExperienceModule::InitializeModule_Implementation(ARTS_Actor* InOwner)
{
	Super::InitializeModule_Implementation(InOwner);

	// Reset runtime state on initialization
	CurrentXP = 0;
	CurrentLevel = 1;
}

void UExperienceModule::AddExperience(int32 Amount)
{
	CurrentXP += Amount;
	OnExperienceGained.Broadcast(Amount);
	OnExperienceUpdate.Broadcast(CurrentXP, XPRequirements.Contains(CurrentLevel) ? XPRequirements[CurrentLevel] : 0);

	while (XPRequirements.Contains(CurrentLevel) && CurrentXP >= XPRequirements[CurrentLevel])
	{
		LevelUp();
		OnExperienceUpdate.Broadcast(CurrentXP, XPRequirements.Contains(CurrentLevel) ? XPRequirements[CurrentLevel] : 0);
	}
}

void UExperienceModule::LevelUp()
{
	if (CurrentLevel >= MaxLevel) return;

	int32 RequiredXP = XPRequirements.Contains(CurrentLevel) ? XPRequirements[CurrentLevel] : 0;
	CurrentXP -= RequiredXP;
	CurrentLevel++;

	UE_LOG(LogTemp, Warning, TEXT("Leveled Up! New Level: %d"), CurrentLevel);

	OnLevelUp.Broadcast(CurrentLevel);
	OnExperienceUpdate.Broadcast(CurrentXP, XPRequirements.Contains(CurrentLevel) ? XPRequirements[CurrentLevel] : 0);
}

int32 UExperienceModule::GetCurrentLevel() const
{
	return CurrentLevel;
}

int32 UExperienceModule::GetXPToNextLevel() const
{
	if (XPRequirements.Contains(CurrentLevel))
	{
		return XPRequirements[CurrentLevel] - CurrentXP;
	}
	return 0;
} 