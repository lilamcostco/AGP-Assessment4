// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "AGP/Components/WeaponComponent.h"
#include "AGP/Pickups/WeaponPickup.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class UHealthComponent;

UCLASS()
class AGP_API ABaseCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, Replicated)
	UWeaponComponent* WeaponComponent;
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* BulletStartPosition;
	
	UPROPERTY(VisibleAnywhere)
	UHealthComponent* HealthComponent;
	
	UPROPERTY(VisibleAnywhere)
	FGenericTeamId TeamID;
	
	void Fire(const FVector& FireAtLocation);
	
	void Reload();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	/// <summary>
	/// Does the player currently have a weapon?
	/// </summary>
	UFUNCTION(BlueprintCallable)
	bool HasWeapon();
	
	/// <summary>
	/// Use to toggle WeaponMesh visibility.
	/// </summary>
	UFUNCTION(BlueprintImplementableEvent)
	void EquipWeaponGraphical(bool bIsWeaponVisible, const EWeaponRarity WeaponRarity = EWeaponRarity::Common);
	
	/// <summary>
	/// Changes the player's weapon state.
	/// </summary>
	bool EquipWeapon(bool bIsEquipNewWeapon, const FWeaponStats& NewWeaponStats, const EWeaponRarity& NewWeaponRarity);
	
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;
	
	virtual FGenericTeamId GetGenericTeamId() const override;
	
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	
private:
	bool EquipWeaponImplementation(bool bIsEquipNewWeapon, const FWeaponStats& NewWeaponStats = FWeaponStats(), const EWeaponRarity& NewWeaponRarity = EWeaponRarity::Common);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEquipWeapon(bool bIsEquipNewWeapon, const EWeaponRarity& NewWeaponRarity);
	
};
