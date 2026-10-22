// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupRotatorComponent.h"

// Sets default values for this component's properties
UPickupRotatorComponent::UPickupRotatorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPickupRotatorComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

/// <summary>
/// Rotates the actors around its z-axis.
/// </summary>
/// <param name="DeltaTime">Time since the last Tick call.</param>
void UPickupRotatorComponent::TickRotateObject(const float& DeltaTime)
{
	// An actor component should always have an actor it is attached to.
	// Actor components themselves do not have transforms, therefore we need the transform of the owning actor.
	FRotator CurrentActorRotation = GetOwner()->GetActorRotation();
	CurrentActorRotation.Yaw += DeltaTime * RotationSpeed;
	GetOwner()->SetActorRotation(CurrentActorRotation);
	// The pattern of get -> change -> set is used here as
	// GetOwner()->Rotation.Yaw += DeltaTime * RotationSpeed;
	// is not valid as the properties are private.
}


// Called every frame
void UPickupRotatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickRotateObject(DeltaTime);
}

