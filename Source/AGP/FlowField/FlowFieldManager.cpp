// Fill out your copyright notice in the Description page of Project Settings.


#include "FlowFieldManager.h"

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
	
	GetWorldTimerManager().SetTimer(GoalTimer, this, &AFlowFieldManager::SetGoalLocation, GoalUpdateInterval, true);
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

