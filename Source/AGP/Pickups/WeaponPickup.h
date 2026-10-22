// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "AGP/Components/WeaponComponent.h"
#include "WeaponPickup.generated.h"

// struct FWeaponStats;

UENUM(BlueprintType)
enum class EWeaponRarity:uint8 
{
	Common,
	Rare,
	Master,
	Legendary
};


UCLASS()
class AGP_API AWeaponPickup : public APickupBase
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	void DetermineWeaponRarity();
	void DetermineWeaponStats();
	void GenerateWeaponPickup();
	
protected:
	UPROPERTY(BlueprintReadOnly, Replicated)
	EWeaponRarity WeaponRarity = EWeaponRarity::Common;
	
	UPROPERTY(Replicated)
	FWeaponStats WeaponStats;
	
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateWeaponPickupMaterial();
	
	virtual void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
