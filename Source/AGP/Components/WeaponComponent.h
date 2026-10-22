// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponComponent.generated.h"

enum class EWeaponRarity : uint8;

UENUM(BlueprintType)
enum class EWeaponType:uint8
{
	Rifle,
	Pistol
};

USTRUCT(BlueprintType)
struct FWeaponStats
{
	GENERATED_BODY()
	
public:
	EWeaponType WeaponType = EWeaponType::Rifle;
	float Accuracy = 1.0f;
	float FireRate = 0.2f;
	UPROPERTY(VisibleAnywhere)
	float BaseDamage = 10.0f;
	int32 MagazineSize = 5;
	float ReloadTime = 1.0f;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AGP_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponComponent();
	
	// Called when the game starts
	virtual void BeginPlay() override;

	bool SetWeaponStats(const FWeaponStats& NewWeaponStats, EWeaponRarity NewWeaponRarity);
	
	EWeaponRarity GetWeaponRarity() const {return WeaponRarity;};
	
	bool IsCurrentWeaponRarer(const EWeaponRarity& NewWeaponRarity) const
	{
		return WeaponRarity > NewWeaponRarity;
	}
	
	// virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere)
	FWeaponStats WeaponStats;
	
	// UPROPERTY(Replicated, VisibleAnywhere)
	EWeaponRarity WeaponRarity;
	
	int32 RoundsRemainingInMagazine;
	float TimeSinceLastShot;
	float ReloadTimer;
	
	void StartReload();
	void EndReload();
	bool IsReloading() const;
	

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	FVector ApplyAccuracyToShot(const FVector& BulletStart, const FVector& FireAtLocation) const;

	/* Fire function that is called externally to make the weapon fire.
	 */
	void Fire(const FVector& BulletStart, const FVector& FireAtLocation);
	
	void Reload();
	
	bool IsMagazineEmpty() const;
	
	FWeaponStats GetWeaponStats() const;
	
private:
	/* Implementation of weapon firing logic. This should only be called on the server via ServerFire and Fire functions.
	 */
	bool FireImplementation(const FVector& BulletStart, const FVector& FireAtLocation, FVector& OutHitLocation);
	void FireVisualImplementation(const FVector& BulletStart, const FVector& FireAtLocation);
	
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFireVisual(const FVector& BulletStart, const FVector& HitLocation);
	
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector& BulletStart, const FVector& FireAtLocation);
	
	void ReloadImplementation();
	
	UFUNCTION(Server, Reliable)
	void ServerReload();
	
};
