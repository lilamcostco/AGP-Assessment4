// Fill out your copyright notice in the Description page of Project Settings.


#include "FlowFieldManager.h"

#include "AGP/Characters/EnemyCharacter.h"
#include "AGP/Components/AINavigationComponent.h"

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
	if (!HasAuthority() || !FlowGrid) return;
	
	TArray<FIntPoint> GoalCells;
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PC = It->Get();
		const APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		if (!Pawn) continue;
		
		const FIntPoint Cell = FlowGrid->WorldToCell(Pawn->GetActorLocation());
		if (FlowGrid->GetCost(Cell) != AFlowFieldGrid::CostBlocked)
		{
			GoalCells.AddUnique(Cell);
		}
	}
	
	if (GoalCells.IsEmpty() || GoalCells == LastGoalCells) return;
	
	if (FlowGrid->SetGoals(GoalCells))
	{
		LastGoalCells = GoalCells;
		if (bDrawFlowDebug)
		{
			FlushPersistentDebugLines(GetWorld());
			FlowGrid->DrawDebugFlowField(0.f);
		}
	}
	
}

void AFlowFieldManager::RegisterEnemy(UAINavigationComponent* Enemy)
{
	if (Enemy)
	{
		Enemies.AddUnique(Enemy);
	}
}

void AFlowFieldManager::UnregisterEnemy(UAINavigationComponent* Enemy)
{
	Enemies.Remove(Enemy);
}

void AFlowFieldManager::UpdateEnemyStates()
{
	// Gather player positions
	TArray<FVector> PlayerLocations;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PC = It->Get();
		if (const APawn* Pawn = PC ? PC->GetPawn() : nullptr)
		{
			PlayerLocations.Add(Pawn->GetActorLocation());
		}
	}
	
	// Removes dead entries from the enemies array
	Enemies.RemoveAll([](const TWeakObjectPtr<UAINavigationComponent>& E) {return !E.IsValid(); });
	
	
	for (const TWeakObjectPtr<UAINavigationComponent>& Enemy : Enemies)
	{
		Enemy->UpdateStateFromPlayers(PlayerLocations);
	}
}

// Called when the game starts or when spawned
void AFlowFieldManager::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority()) return; // only generates on the server
	
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
	LastGoalCells.Reset();
	SetGoalLocation(); // preliminary set goal
	
	// repeatedly triggers the set goal location according to the set interval
	GetWorldTimerManager().SetTimer(GoalTimer, this, &AFlowFieldManager::SetGoalLocation, GoalUpdateInterval, true);
	GetWorldTimerManager().SetTimer(StateTimer, this, &AFlowFieldManager::UpdateEnemyStates, StateUpdateInterval, true);
}