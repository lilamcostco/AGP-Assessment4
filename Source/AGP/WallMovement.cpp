// Fill out your copyright notice in the Description page of Project Settings.


#include "WallMovement.h"

// Sets default values
AWallMovement::AWallMovement()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MoveSpeed = 50.0f; 
    MoveDirection = FVector(1.0f, 0.0f, 0.0f);

}

// Called when the game starts or when spawned
void AWallMovement::BeginPlay()
{
	Super::BeginPlay();
	MoveDirection.Normalize();
}

// Called every frame
void AWallMovement::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector CurrentLocation = GetActorLocation();
    FVector NewLocation = CurrentLocation + (MoveDirection * MoveSpeed * DeltaTime);
    SetActorLocation(NewLocation);
}

