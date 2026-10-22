// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"

#include "AGP/Characters/BaseCharacter.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	SetIsReplicated(true);
	PrimaryComponentTick.bCanEverTick = true;

	TimeSinceLastShot = 0.0f;
}

bool UWeaponComponent::SetWeaponStats(const FWeaponStats& NewWeaponStats, const EWeaponRarity NewWeaponRarity)
{
	UE_LOG(LogTemp, Warning, TEXT("Current Weapon Rarity: %i, New Weapon Rarity: %i, Is new better? %i"), WeaponRarity, NewWeaponRarity, IsCurrentWeaponRarer(NewWeaponRarity) ? 0 : 1)
	if (IsCurrentWeaponRarer(NewWeaponRarity)) return false;
	WeaponRarity = NewWeaponRarity;
	WeaponStats = NewWeaponStats;
	return true;
}

// void UWeaponComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
// {
// 	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
// 	DOREPLIFETIME(UWeaponComponent, WeaponRarity);
// }


// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastShot += DeltaTime;
	
	if (IsReloading())
	{
		ReloadTimer -= DeltaTime;
		if (ReloadTimer <= 0.0f)
		{
			EndReload();
		}
	}
}

FVector UWeaponComponent::ApplyAccuracyToShot(const FVector& BulletStart, const FVector& FireAtLocation) const
{
	FVector Direction;
	float Distance;
	(FireAtLocation - BulletStart).ToDirectionAndLength(Direction, Distance);
	FVector RandomDirection = FMath::VRand();
	FVector AccuracyAffectedDirection = FMath::Lerp(RandomDirection, Direction, WeaponStats.Accuracy);
	FVector AccuracyAffectedFireAtLocation = BulletStart + Distance*AccuracyAffectedDirection;
	return AccuracyAffectedFireAtLocation;
}

void UWeaponComponent::Fire(const FVector& BulletStart, const FVector& FireAtLocation)
{
	// If on the server, then perform the FireImplementation locally. 
	if (const ENetRole OwnerLocalRole = GetOwner()->GetLocalRole(); OwnerLocalRole==ROLE_Authority)
	{
		if (FVector HitLocation; FireImplementation(BulletStart, FireAtLocation, HitLocation))
		{
			MulticastFireVisual(BulletStart, HitLocation);
		}
	}
	// if locally controlled, but not a server, then tell the server to fire a bullet instead.
	else if (OwnerLocalRole==ROLE_AutonomousProxy)
	{
		ServerFire(BulletStart, FireAtLocation);
	}
	
}

void UWeaponComponent::Reload()
{
	if (ENetRole OwnerLocalRole = GetOwner()->GetLocalRole(); OwnerLocalRole==ROLE_Authority)
	{
		ReloadImplementation();
	}
	else if (OwnerLocalRole==ROLE_AutonomousProxy)
	{
		ServerReload();
	}
}

void UWeaponComponent::StartReload()
{
	if (IsReloading()) return;
	ReloadTimer = WeaponStats.ReloadTime;
}

bool UWeaponComponent::IsMagazineEmpty() const
{
	return RoundsRemainingInMagazine <= 0;
}

FWeaponStats UWeaponComponent::GetWeaponStats() const
{
	return WeaponStats;
}

bool UWeaponComponent::FireImplementation(const FVector& BulletStart, const FVector& FireAtLocation,
	FVector& OutHitLocation)
{
	if (TimeSinceLastShot < WeaponStats.FireRate) return false;
	if (IsMagazineEmpty()) return false;
	if (IsReloading()) return false;
	
	TimeSinceLastShot = 0.0f;
	RoundsRemainingInMagazine = FMath::Max(0, RoundsRemainingInMagazine - 1);
	
	FHitResult HitResult;
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.AddIgnoredActor(GetOwner());

	const FVector AccuracyAffectedFireAtLocation = ApplyAccuracyToShot(BulletStart, FireAtLocation);
	
	if (GetWorld()->LineTraceSingleByChannel(
		HitResult, 
		BulletStart, 
		AccuracyAffectedFireAtLocation, 
		ECC_Pawn, 
		CollisionParameters))
	{
		OutHitLocation = HitResult.Location;
		if (ABaseCharacter* Character = Cast<ABaseCharacter>(HitResult.GetActor()))
		{
			Character->TakeDamage(WeaponStats.BaseDamage,FDamageEvent(), Cast<ABaseCharacter>(GetOwner())->GetController(), GetOwner());
			DrawDebugLine(
				GetWorld(),
				BulletStart, HitResult.ImpactPoint,
				FColor::Green,false,
				1.0f,0,3.0f
				);
		}
		else
		{
			DrawDebugLine(
				GetWorld(),
				BulletStart, HitResult.ImpactPoint,
				FColor::Orange,false,
				1.0f,0,3.0f
				);
		}
	}
	else
	{
		OutHitLocation = AccuracyAffectedFireAtLocation;
		DrawDebugLine(
				GetWorld(),
				BulletStart, AccuracyAffectedFireAtLocation,
				FColor::Red,false,
				1.0f,0,3.0f
				);
	}
	
	return true;
}

void UWeaponComponent::FireVisualImplementation(const FVector& BulletStart, const FVector& FireAtLocation)
{
	DrawDebugLine(GetWorld(),BulletStart,FireAtLocation, FColor::Blue,false, 1.0f, 0, 3.0f);
}

void UWeaponComponent::ReloadImplementation()
{
	StartReload();
}

void UWeaponComponent::ServerReload_Implementation()
{
	ReloadImplementation();
}

void UWeaponComponent::ServerFire_Implementation(const FVector& BulletStart, const FVector& FireAtLocation)
{
	if (FVector HitLocation; FireImplementation(BulletStart, FireAtLocation, HitLocation))
	{
		MulticastFireVisual(BulletStart, HitLocation);
	}
}

void UWeaponComponent::MulticastFireVisual_Implementation(const FVector& BulletStart, const FVector& HitLocation)
{
	FireVisualImplementation(BulletStart, HitLocation);
}

void UWeaponComponent::EndReload()
{
	RoundsRemainingInMagazine = WeaponStats.MagazineSize;
	ReloadTimer = 0.0f;
}

bool UWeaponComponent::IsReloading() const
{
	return ReloadTimer > 0.0f;
}

