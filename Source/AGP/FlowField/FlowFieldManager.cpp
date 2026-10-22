// Fill out your copyright notice in the Description page of Project Settings.


#include "FlowFieldManager.h"

#include "EngineUtils.h"
#include "SWarningOrErrorBox.h"
#include "AGP/Characters/PlayerCharacter.h"
#include "AGP/Components/AINavigationComponent.h"
#include "Kismet/GameplayStatics.h"

class APlayerCharacter;
// Sets default values
AFlowFieldManager::AFlowFieldManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	
}

void AFlowFieldManager::SetGoalLocation()
{
	if (!FlowGrid) return;
	
	if (!IsValid(Target))
	{
		Target = FindPlayer();
		if (!Target) return;
	}
	
	const FVector TargetLocation = Target->GetActorLocation();
	const FIntPoint Cell = FlowGrid->WorldToCell(TargetLocation);
	
	// making sure the Goal Cell isn't the same as before for efficiency
	if (Cell == LastGoalCell) return;
	
	if (FlowGrid->SetGoal(TargetLocation))
	{
		LastGoalCell = Cell;
		
		if (bDrawFlowDebug)
		{
			FlowGrid->DrawDebugFlowField(GoalUpdateInterval);
		}
		// Updating All the enemies Flow Field
		for (AEnemyCharacter* Enemy : Enemies)
		{
			Enemy->FindComponentByClass<UAINavigationComponent>()->UpdateFlowField(FlowGrid);
		}
	}
}

// Called when the game starts or when spawned
void AFlowFieldManager::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority()) return;
	
	// setting the FlowGrid to be the actor attached to the manager
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	
	for (AActor* ChildActor : AttachedActors)
	{
		if (AFlowFieldGrid* Grid = Cast<AFlowFieldGrid>(ChildActor))
		{
			FlowGrid = Grid;
			break;
		}
	}
	
	if (!FlowGrid)
	{
		UE_LOG(LogTemp, Error, TEXT("FlowFieldManager: no FlowFieldGrid attached to %s"), *GetName());
		return;
	}
	
	
	FlowGrid->BuildGrid();      // guarantees the origin and cost field exist
	Target = FindPlayer();
	
	GetWorldTimerManager().SetTimer(GoalTimer, this, &AFlowFieldManager::SetGoalLocation, GoalUpdateInterval, true);
}

APlayerCharacter* AFlowFieldManager::FindPlayer()
{
	//Only works for single player, NEED TO CHANGE FOR MULTIPLAYER
	return Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
}

void AFlowFieldManager::SetEnemies()
{
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	
	for (AActor* ChildActor : AttachedActors)
	{
		if (AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(ChildActor))
		{
			Enemies.Add(Enemy);
		}
	}
}

