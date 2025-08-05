// Copyright AmberleafCotton 2025. All Rights Reserved.
#include "RecruitmentModule.h"
#include "RTS_Actor.h"
#include "Kismet/GameplayStatics.h"

URecruitmentModule::URecruitmentModule()
{
}

void URecruitmentModule::InitializeModule_Implementation(ARTS_Actor* InOwner)
{
	Super::InitializeModule_Implementation(InOwner);
}

void URecruitmentModule::AddUnitToProduction(UUnitDataAsset* UnitData)
{
	if (!UnitData) return;

	UnitProductionQueue.Add(UnitData);

	if (GetWorld()->GetTimerManager().IsTimerActive(ProductionTimerHandle))
	{
		OnProductionQueueUpdated.Broadcast(UnitProductionQueue);
	}
	else
	{
		EnableProduction();
	}
}

void URecruitmentModule::EnableProduction()
{
	if (!GetWorld()) return;

	GetWorld()->GetTimerManager().SetTimer(
		ProductionTimerHandle,
		this,
		&URecruitmentModule::ProcessProductionQueue,
		ProductionTimerGranularity,
		true
	);
}

void URecruitmentModule::ProcessProductionQueue()
{
	if (!bIsProducingUnit)
	{
		if (UnitProductionQueue.Num() > 0 && UnitProductionQueue.IsValidIndex(0))
		{
			UnitBeingProduced = UnitProductionQueue[0];
			ProductionTimeNeeded = UnitBeingProduced->ProductionData.ProductionTime;
			ProductionTimeSpent = 0.0f;
			ProductionProgress = 0.0f;
			bIsProducingUnit = true;
		}
		return;
	}

	ProductionTimeSpent += ProductionTimerGranularity;
	ProductionProgress = ProductionTimeSpent / ProductionTimeNeeded;

	OnProductionProgressUpdated.Broadcast(ProductionProgress);

	if (ProductionProgress >= 1.0f)
	{
		SpawnUnit();

		ProductionTimeSpent = 0.0f;
		ProductionProgress = 0.0f;

		UnitProductionQueue.RemoveAt(0);
		OnProductionQueueUpdated.Broadcast(UnitProductionQueue);

		bIsProducingUnit = false;
		UnitBeingProduced = nullptr;

		if (UnitProductionQueue.Num() <= 0)
		{
			GetWorld()->GetTimerManager().ClearTimer(ProductionTimerHandle);
			OnProductionProgressUpdated.Broadcast(ProductionProgress);
			OnProductionQueueUpdated.Broadcast(UnitProductionQueue);
		}
	}
}

void URecruitmentModule::SpawnUnit_Implementation()
{
	if (!UnitBeingProduced || !Owner || !UnitBeingProduced->UnitClass) return;

	FVector SpawnLocation = Owner->GetActorLocation();
	FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* DeferredUnit = GetWorld()->SpawnActorDeferred<AActor>(
		UnitBeingProduced->UnitClass,
		FTransform(SpawnRotation, SpawnLocation),
		Owner,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	if (!DeferredUnit) return;

	// Placeholder for unit/team initialization before finishing spawn
	// e.g., IInitializableUnitInterface::Execute_Initialize(DeferredUnit, TeamID);

	DeferredUnit->FinishSpawning(FTransform(SpawnRotation, SpawnLocation));
}
