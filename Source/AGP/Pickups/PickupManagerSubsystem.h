// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PickupManagerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class AGP_API UPickupManagerSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
protected:
	TArray<FVector> PossibleSpawnLocations;
	
	virtual void Tick(float DeltaTime) override;
	
protected:
	float PickupSpawnRate = 5.0f;
	float TimeSinceLastSpawn = 0.0f;
	
private:
	void PopulateSpawnLocations();
	void SpawnWeaponPickup();
	
public:
	virtual TStatId GetStatId() const override {return TStatId();}
};
