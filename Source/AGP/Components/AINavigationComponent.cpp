// Fill out your copyright notice in the Description page of Project Settings.


#include "AINavigationComponent.h"


#include "AGP/FlowField/FlowFieldManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UAINavigationComponent::UAINavigationComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}

void UAINavigationComponent::UpdateFlowField(AFlowFieldGrid* NewFlowField)
{
    if (NewFlowField)
       FlowField = NewFlowField;
}


// Called when the game starts
void UAINavigationComponent::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void UAINavigationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	MoveEnemy(DeltaTime);
}

AFlowFieldGrid* UAINavigationComponent::GetGrid()
{
	if (IsValid(FlowField))
	{
		return FlowField;
	}

	if (!IsValid(Manager))
	{
		Manager = Cast<AFlowFieldManager>(
			UGameplayStatics::GetActorOfClass(this, AFlowFieldManager::StaticClass()));
	}

	if (Manager)
	{
		UpdateFlowField(Manager->GetGrid());   // stays null until the manager has found its grid
	}
	return FlowField;
}

void UAINavigationComponent::MoveEnemy(float DeltaTime)
{
	
	AActor* Owner = GetOwner();
	if (!Owner || !GetGrid())
	{
		return;   // grid not ready yet, retried next tick
	}
    
	UpdateMoveDirection();
    
    if (MoveDirection.IsNearlyZero())
    {
       return; // At the goal, blocked or unreachable
    }
    
    if (APawn* Pawn = Cast<APawn>(GetOwner()))
    {
       Pawn->AddMovementInput(MoveDirection, 1.f);
    }
}

void UAINavigationComponent::UpdateMoveDirection()
{
    MoveDirection = FlowField->GetFlowDirection(GetOwner()->GetActorLocation());
}