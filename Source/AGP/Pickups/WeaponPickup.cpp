// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponPickup.h"
#include <AGP/Characters/PlayerCharacter.h>

#include "Net/UnrealNetwork.h"

void AWeaponPickup::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeaponPickup, WeaponRarity)
	DOREPLIFETIME(AWeaponPickup, WeaponStats)
}

void AWeaponPickup::DetermineWeaponRarity()
{
	if (const float RarityValue = FMath::RandRange(0.0f, 1.0f); RarityValue <= 0.5f)
	{
		WeaponRarity = EWeaponRarity::Common;
	}
	else if (RarityValue > 0.5f && RarityValue <= 0.8f)
	{
		WeaponRarity = EWeaponRarity::Rare;
	}
	else if (RarityValue > 0.8f && RarityValue <= 0.95f)
	{
		WeaponRarity = EWeaponRarity::Master;
	}
	else
	{
		WeaponRarity = EWeaponRarity::Legendary;
	}
}

void AWeaponPickup::DetermineWeaponStats()
{
	int32 NumGoodStats = 0;

	switch (WeaponRarity)
	{
	case EWeaponRarity::Common:
		{
			NumGoodStats = 0;
			break;
		}
	case EWeaponRarity::Rare:
		{
			NumGoodStats = 2;
			break;
		}
		
	case EWeaponRarity::Master:
		{
			NumGoodStats = 3;
			break;
		}
	case EWeaponRarity::Legendary:
		{
			NumGoodStats = 4;
			break;
		}
	}
	
	TArray<bool> GoodStatArray = {false, false, false, false, false};
	int32 RemainingGoodStats = NumGoodStats;
	while (RemainingGoodStats > 0)
	{
		int32 TestIndex = FMath::RandRange(0, GoodStatArray.Num() - 1);
		if (GoodStatArray[TestIndex]) continue;
		GoodStatArray[TestIndex] = true;
		RemainingGoodStats--;
	}
	
	WeaponStats.Accuracy = GoodStatArray[0] ? FMath::RandRange(0.98f, 1.0f) : FMath::RandRange(0.9f, 0.98f);
	WeaponStats.FireRate = GoodStatArray[1] ? FMath::RandRange(0.05f, 0.2f) : FMath::RandRange(0.2f, 1.0f);
	WeaponStats.BaseDamage = GoodStatArray[2] ? FMath::RandRange(15.0f, 30.0f) : FMath::RandRange(5.0f, 15.0f);
	WeaponStats.MagazineSize = GoodStatArray[3] ? FMath::RandRange(20, 100) : FMath::RandRange(1, 19);
	WeaponStats.ReloadTime = GoodStatArray[4] ? FMath::RandRange(0.1f, 1.0f) : FMath::RandRange(1.0f, 4.0f);
}

void AWeaponPickup::GenerateWeaponPickup()
{
	DetermineWeaponRarity();
	DetermineWeaponStats();
}

void AWeaponPickup::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetLocalRole()==ROLE_Authority)
	{
		GenerateWeaponPickup();
	}
	
	UpdateWeaponPickupMaterial();
}

void AWeaponPickup::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnPickupOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	UE_LOG(LogTemp, Warning, TEXT("Weapon Pickup Overlap Occurred!!!"))

	if (ABaseCharacter* Character = Cast<ABaseCharacter>(OtherActor)) {
		// if (Character->HasWeapon()) {
		// 	return;
		// }
		if (Character->EquipWeapon(true, WeaponStats, WeaponRarity))
		{
			Destroy();
		}
	}
}
