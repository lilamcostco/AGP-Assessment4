// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"

#include "AGP/Components/HealthComponent.h"
#include "AGP/Components/WeaponComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
	
	WeaponComponent = nullptr;
	
	BulletStartPosition = CreateDefaultSubobject<USceneComponent>(TEXT("Bullet Start Position"));
	if (BulletStartPosition)
	{
		BulletStartPosition->SetupAttachment(GetRootComponent());
	}
	
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Make sure weapon visibility is as expected on BeginPlay.
	EquipWeapon(HasWeapon(), FWeaponStats(), EWeaponRarity::Common);
	
	// used to hide initial weapon as client does not exist during server BeginPlay call, resulting in a failed multicast. 
	if (GetLocalRole()!=ROLE_Authority)
	{
		EquipWeaponGraphical(false);
	}
}


void ABaseCharacter::Fire(const FVector& FireAtLocation)
{
	if (!HasWeapon()) return;
	
	WeaponComponent->Fire(BulletStartPosition->GetComponentLocation(), FireAtLocation);
	
}

void ABaseCharacter::Reload()
{
	if (!HasWeapon()) return;
	
	WeaponComponent->Reload();
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseCharacter, WeaponComponent);
	
}

bool ABaseCharacter::HasWeapon()
{
	return WeaponComponent!=nullptr;
}

bool ABaseCharacter::EquipWeapon(bool bIsEquipNewWeapon, const FWeaponStats& NewWeaponStats, const EWeaponRarity& NewWeaponRarity)
{
	if (GetLocalRole()==ROLE_Authority)
	{
		const bool bWasEquipSuccessful = EquipWeaponImplementation(bIsEquipNewWeapon, NewWeaponStats, NewWeaponRarity);
		if (bWasEquipSuccessful || !bIsEquipNewWeapon)
		{
			MulticastEquipWeapon(bIsEquipNewWeapon, NewWeaponRarity);
		}
		return bWasEquipSuccessful;
	}
	return false;
}

void ABaseCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	this->TeamID=NewTeamID;
}

FGenericTeamId ABaseCharacter::GetGenericTeamId() const
{
	return TeamID;
}

ETeamAttitude::Type ABaseCharacter::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(&Other))
	{
		if (GetGenericTeamId() == OtherTeamAgent->GetGenericTeamId())
		{
			return ETeamAttitude::Friendly;
		}
		return ETeamAttitude::Hostile;
	}
	return ETeamAttitude::Neutral;
}


bool ABaseCharacter::EquipWeaponImplementation(bool bIsEquipNewWeapon, const FWeaponStats& NewWeaponStats, const EWeaponRarity& NewWeaponRarity)
{
	if (bIsEquipNewWeapon && !HasWeapon())
	{
		WeaponComponent = NewObject<UWeaponComponent>(this);
		WeaponComponent->RegisterComponent();
	}
	else if (!bIsEquipNewWeapon && HasWeapon())
	{
		WeaponComponent->UnregisterComponent();
		WeaponComponent = nullptr;
	}
	
	if (HasWeapon())
	{
		const bool bDidWeaponEquip = WeaponComponent->SetWeaponStats(NewWeaponStats, NewWeaponRarity);
		if (bDidWeaponEquip)
		{
			Reload();
		}
		EquipWeaponGraphical(HasWeapon(), WeaponComponent->GetWeaponRarity());
		return bDidWeaponEquip;
	}
	
	// Should only be called if no weapon is now equipped 
	EquipWeaponGraphical(HasWeapon());
	
	return false;
}

void ABaseCharacter::MulticastEquipWeapon_Implementation(bool bIsEquipNewWeapon, const EWeaponRarity& NewWeaponRarity)
{
	// EquipWeaponImplementation(bIsEquipNewWeapon, NewWeaponStats);
	UE_LOG(LogTemp, Display, TEXT("Trying to show weapon on client: %i, Are we on server? %i"), bIsEquipNewWeapon ? 1 : 0, (GetLocalRole()==ROLE_Authority) ? 1 : 0);
	EquipWeaponGraphical(bIsEquipNewWeapon, NewWeaponRarity);
}

