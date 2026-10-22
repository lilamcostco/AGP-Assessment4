// Fill out your copyright notice in the Description page of Project Settings.


#include "FlowFieldGrid.h"

#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AFlowFieldGrid::AFlowFieldGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

// Called when the game starts or when spawned
void AFlowFieldGrid::BeginPlay()
{
	Super::BeginPlay();
	
	if (bBuildOnBeginPlay)
	{
		BuildGrid();
	}
}

void AFlowFieldGrid::BuildGrid()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Making sure to refresh the cells and the solver
	const int32 NumCells = GridDimensions.X * GridDimensions.Y;
	Solver.Reset();
	CostField.Init(CostBlocked, NumCells);
	CellHeights.Init(0.f, NumCells);


	// Actor location is the grid center. Compute the min corner.
	const FVector Center = GetActorLocation();
	GridOrigin = FVector(
		Center.X - (GridDimensions.X * CellSize) * 0.5f,
		Center.Y - (GridDimensions.Y * CellSize) * 0.5f,
		Center.Z);
	
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FlowFieldBuild), /*bTraceComplex=*/false, this);
	
	for (const TSubclassOf<AActor>& ActorClass : IgnoredActorClasses)
	{
		if (!*ActorClass)
		{
			continue;
		}

		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsOfClass(World, ActorClass, Found);
		QueryParams.AddIgnoredActors(Found);
	}
	
	const float MinNormalZ = FMath::Cos(FMath::DegreesToRadians(MaxSlopeAngle));
	const FCollisionShape ProbeShape = FCollisionShape::MakeSphere(AgentRadius);

	UNavigationSystemV1* NavSys = bRequireNavMesh ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;

	int32 WalkableCount = 0;

	for (int32 Y = 0; Y < GridDimensions.Y; ++Y)
	{
		for (int32 X = 0; X < GridDimensions.X; ++X)
		{
			const FIntPoint Cell(X, Y);
			const int32 Index = CellToIndex(Cell);

			const FVector CellXY(
				GridOrigin.X + (X + 0.5f) * CellSize,
				GridOrigin.Y + (Y + 0.5f) * CellSize,
				GridOrigin.Z);

			CellHeights[Index] = GridOrigin.Z;

			// 1. Find the ground.
			const FVector TraceStart = CellXY + FVector(0.f, 0.f, TraceHalfHeight);
			const FVector TraceEnd = CellXY - FVector(0.f, 0.f, TraceHalfHeight);

			FHitResult GroundHit;
			if (!World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, GroundChannel, QueryParams))
			{
				continue; // No ground: blocked
			}

			CellHeights[Index] = GroundHit.ImpactPoint.Z;

			// 2. Slope check.
			if (GroundHit.ImpactNormal.Z < MinNormalZ)
			{
				continue;
			}

			// 3. Obstacle check: a sphere floating StepHeight above the ground, so the floor
			//    and small steps are ignored but walls and props are caught.
			const FVector ProbeCenter = GroundHit.ImpactPoint + FVector(0.f, 0.f, StepHeight + AgentRadius);
			if (World->OverlapBlockingTestByChannel(ProbeCenter, FQuat::Identity, ObstacleChannel, ProbeShape, QueryParams))
			{
				continue;
			}

			// 4. Optional nav mesh check.
			if (bRequireNavMesh)
			{
				FNavLocation NavLoc;
				if (!NavSys || !NavSys->ProjectPointToNavigation(GroundHit.ImpactPoint, NavLoc, NavQueryExtent))
				{
					continue;
				}
			}

			CostField[Index] = DefaultCost;
			++WalkableCount;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("FlowFieldGrid '%s': built %dx%d grid, %d/%d cells walkable"),
		*GetName(), GridDimensions.X, GridDimensions.Y, WalkableCount, NumCells);

	if (bDrawDebugOnBuild)
	{
		DrawDebugGrid(10.f);
	}
}

void AFlowFieldGrid::DrawDebugGrid(float Duration) const
{
	UWorld* World = GetWorld();
	if (!World || CostField.Num() != GridDimensions.X * GridDimensions.Y)
	{
		return;
	}

	const bool bPersistent = Duration <= 0.f;
	const FVector Extent(CellSize * 0.45f, CellSize * 0.45f, 2.f);

	for (int32 Index = 0; Index < CostField.Num(); ++Index)
	{
		const FIntPoint Cell = IndexToCell(Index);
		const bool bBlocked = CostField[Index] == CostBlocked;

		DrawDebugBox(
			World,
			CellToWorld(Cell) + FVector(0.f, 0.f, 5.f),
			Extent,
			bBlocked ? FColor::Red : FColor::Green,
			bPersistent,
			bPersistent ? -1.f : Duration,
			0,
			1.f);
	}
}

bool AFlowFieldGrid::IsValidCell(const FIntPoint& Cell) const
{
	return Cell.X >= 0 && Cell.Y >= 0 && Cell.X < GridDimensions.X && Cell.Y < GridDimensions.Y;
}


FVector AFlowFieldGrid::CellToWorld(const FIntPoint& Cell) const
{
	const float Z = (IsValidCell(Cell) && CellHeights.IsValidIndex(CellToIndex(Cell)))
		? CellHeights[CellToIndex(Cell)]
		: GridOrigin.Z;

	return FVector(
		GridOrigin.X + (Cell.X + 0.5f) * CellSize,
		GridOrigin.Y + (Cell.Y + 0.5f) * CellSize,
		Z);
}

FIntPoint AFlowFieldGrid::WorldToCell(const FVector& WorldLocation) const
{
	return FIntPoint(
		FMath::FloorToInt((WorldLocation.X - GridOrigin.X) / CellSize),
		FMath::FloorToInt((WorldLocation.Y - GridOrigin.Y) / CellSize));
}


uint8 AFlowFieldGrid::GetCost(const FIntPoint& Cell) const
{
	if (!IsValidCell(Cell) || !CostField.IsValidIndex(CellToIndex(Cell)))
	{
		return CostBlocked;
	}
	return CostField[CellToIndex(Cell)];
}

bool AFlowFieldGrid::IsWalkable(const FVector& WorldLocation) const
{
	return GetCost(WorldToCell(WorldLocation)) != CostBlocked;
}

// ------- Flow Field (delegates) --------

bool AFlowFieldGrid::SetGoal(const FVector& GoalWorldLocation)
{
	if (CostField.Num() != GridDimensions.X * GridDimensions.Y)
	{
		return false;
	}

	const FIntPoint GoalCell = WorldToCell(GoalWorldLocation);
	if (GetCost(GoalCell) == CostBlocked)
	{
		return false;
	}

	return SetGoals({ GoalCell });
}

bool AFlowFieldGrid::SetGoals(const TArray<FIntPoint>& GoalCells)
{
	if (CostField.Num() != GridDimensions.X * GridDimensions.Y)
	{
		return false;
	}

	return Solver.Solve(GridDimensions, CostField, GoalCells);
}

FVector AFlowFieldGrid::GetFlowDirection(const FVector& WorldLocation) const
{
	const FVector2D Dir = Solver.GetDirection(WorldToCell(WorldLocation));
	return FVector(Dir.X, Dir.Y, 0.f);
}

void AFlowFieldGrid::DrawDebugFlowField(float Duration) const
{
	UWorld* World = GetWorld();
	if (!World || !Solver.HasData())
	{
		return;
	}

	const bool bPersistent = Duration <= 0.f;
	const float ArrowLength = CellSize * 0.4f;
	const TArray<FVector2D>& Flow = Solver.GetFlowField();

	for (int32 Index = 0; Index < Flow.Num(); ++Index)
	{
		const FVector2D Dir = Flow[Index];
		const FIntPoint Cell = IndexToCell(Index);
		const FVector Start = CellToWorld(Cell) + FVector(0.f, 0.f, 10.f);

		if (Dir.IsNearlyZero())
		{
			// Goal cells get a blue dot, and blocked or unreachable cells get nothing.
			if (Solver.GetIntegration(Cell) == 0)
			{
				DrawDebugPoint(World, Start, 12.f, FColor::Blue, bPersistent, bPersistent ? -1.f : Duration);
			}
			continue;
		}

		const FVector End = Start + FVector(Dir.X, Dir.Y, 0.f) * ArrowLength;
		DrawDebugDirectionalArrow(World, Start, End, 25.f, FColor::Cyan, bPersistent,
			bPersistent ? -1.f : Duration, 0, 2.f);
	}
}
