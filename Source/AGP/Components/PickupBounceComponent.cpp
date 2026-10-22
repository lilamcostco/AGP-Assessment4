// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupBounceComponent.h"

// Sets default values for this component's properties
UPickupBounceComponent::UPickupBounceComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPickupBounceComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache the initial starting position
	InitialLocation = GetOwner()->GetActorLocation();
	bIsMovingUp = true;
}

/// <summary>
/// Bounce the pickup linearly around the initial position.
/// </summary>
/// <param name="DeltaTime">Time that has passed in seconds since the last frame.</param>
void UPickupBounceComponent::TickBounceObject(const float& DeltaTime)
{
	// Initialize CurrentLocation, InitialHeight, and Direction of Movement.
	FVector CurrentActorLocation = GetOwner()->GetActorLocation();
	float InitialHeight = InitialLocation.Z;
	float BounceDirection = bIsMovingUp ? 1.0f : -1.0f;

	// Calculate the new height.
	float NewHeight = CurrentActorLocation.Z + DeltaTime * BounceSpeed * BounceDirection;
	
	//Clamp the new height to BounceDistance, and change direction.
	if (abs(NewHeight - InitialHeight) >= BounceDistance) {
		NewHeight = InitialHeight + BounceDirection * BounceDistance;
		bIsMovingUp = !bIsMovingUp;
	}
	CurrentActorLocation.Z = NewHeight;

	// Set the actor to the new position.
	GetOwner()->SetActorLocation(CurrentActorLocation);
}

/// <summary>
/// Bounce the Pickup object around it's initial starting position using a Sin function.
/// </summary>
void UPickupBounceComponent::TickSinBounceObject()
{
	FVector ActorLocation = GetOwner()->GetActorLocation();
	ActorLocation.Z = InitialLocation.Z + BounceDistance * sinf(BounceSpeed/100.0f * GetWorld()->TimeSeconds);
	GetOwner()->SetActorLocation(ActorLocation);
}


// Called every frame
void UPickupBounceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickBounceObject(DeltaTime);
}

