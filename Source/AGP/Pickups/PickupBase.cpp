// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupBase.h"

#include <Components/StaticMeshComponent.h>
#include <Components/BoxComponent.h>

// Sets default values
APickupBase::APickupBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;

	// Create the child objects.
	PickupCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Pickup Collider"));
	PickupMesh= CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pickup Mesh"));

	// Set collider as the scene root.
	if (PickupCollider) {
		SetRootComponent(PickupCollider);
	}

	// Attach mesh to the current scene root.
	if (PickupMesh) {
		PickupMesh->SetupAttachment(GetRootComponent());
	}
}

// Called when the game starts or when spawned
void APickupBase::BeginPlay()
{
	Super::BeginPlay();
	
	// Bind the OnPickupOverlap function to the pickup collider's OnComponentBeginOverlap event as a dynamic delegate as the event is broadcast.
	if (PickupCollider) {
		PickupCollider->OnComponentBeginOverlap.AddDynamic(this, &APickupBase::OnPickupOverlap);
	}
}

// Called every frame
void APickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickupBase::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("Overlap event occurred in PickupBase"))
}

