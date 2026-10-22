// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FFlowFieldSolver.h"
#include "GameFramework/Actor.h"
#include "FlowFieldGrid.generated.h"

UCLASS()
class AGP_API AFlowFieldGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlowFieldGrid();
	
	static constexpr uint8 CostBlocked = FFlowFieldSolver::CostBlocked;

	// ---- Grid layout ----

	/** Number of cells along X and Y. */
	UPROPERTY(EditAnywhere, Category = "Grid", meta = (ClampMin = "1", ClampMax = "2048"))
	FIntPoint GridDimensions = FIntPoint(100, 100);

	/** Cell edge length in Unreal units. Roughly agent diameter or larger. */
	UPROPERTY(EditAnywhere, Category = "Grid", meta = (ClampMin = "10.0"))
	float CellSize = 100.f;

	// ---- Ground detection ----

	/** Channel used for the downward ground trace. */
	UPROPERTY(EditAnywhere, Category = "Grid|Ground")
	TEnumAsByte<ECollisionChannel> GroundChannel = ECC_Visibility;

	/** Trace runs from +TraceHalfHeight to -TraceHalfHeight around the actor's Z. */
	UPROPERTY(EditAnywhere, Category = "Grid|Ground", meta = (ClampMin = "10.0"))
	float TraceHalfHeight = 2000.f;

	/** Surfaces steeper than this (degrees) are blocked. */
	UPROPERTY(EditAnywhere, Category = "Grid|Ground", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float MaxSlopeAngle = 45.f;

	// ---- Obstacle detection ----
	
	/** Actors of these classes (and subclasses) are ignored by every trace and overlap in BuildGrid. */
	UPROPERTY(EditAnywhere, Category = "Grid|Obstacles")
	TArray<TSubclassOf<AActor>> IgnoredActorClasses;
	
	/** Channel used for the obstacle overlap test. Make sure your walls/props block this channel. */
	UPROPERTY(EditAnywhere, Category = "Grid|Obstacles")
	TEnumAsByte<ECollisionChannel> ObstacleChannel = ECC_WorldStatic;

	/** Radius of the sphere used to test for obstacles. */
	UPROPERTY(EditAnywhere, Category = "Grid|Obstacles", meta = (ClampMin = "1.0"))
	float AgentRadius = 40.f;

	/** Obstacles shorter than this above the ground are ignored (curbs, stairs, small props). */
	UPROPERTY(EditAnywhere, Category = "Grid|Obstacles", meta = (ClampMin = "1.0"))
	float StepHeight = 30.f;

	// ---- Nav mesh ----

	/** If true, a cell must also project onto the nav mesh to be walkable. */
	UPROPERTY(EditAnywhere, Category = "Grid|NavMesh")
	bool bRequireNavMesh = false;

	UPROPERTY(EditAnywhere, Category = "Grid|NavMesh", meta = (EditCondition = "bRequireNavMesh"))
	FVector NavQueryExtent = FVector(50.f, 50.f, 100.f);

	// ---- Cost ----

	/** Cost assigned to walkable cells (1..254). */
	UPROPERTY(EditAnywhere, Category = "Grid|Cost", meta = (ClampMin = "1", ClampMax = "254"))
	uint8 DefaultCost = 1;

	// ---- Build / debug ----

	UPROPERTY(EditAnywhere, Category = "Grid|Build")
	bool bBuildOnBeginPlay = false;

	UPROPERTY(EditAnywhere, Category = "Grid|Debug")
	bool bDrawDebugOnBuild = true;

	/** Rebuilds the grid. Also callable from the details panel in the editor. */
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Grid")
	void BuildGrid();

	/** Draws walkable (green) and blocked (red) cells. Duration <= 0 means persistent until flushed. */
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Grid")
	void DrawDebugGrid(float Duration = 10.f) const;

	// ---- Queries ----

	bool IsValidCell(const FIntPoint& Cell) const;
	int32 CellToIndex(const FIntPoint& Cell) const { return Cell.Y * GridDimensions.X + Cell.X; }
	FIntPoint IndexToCell(int32 Index) const { return FIntPoint(Index % GridDimensions.X, Index / GridDimensions.X); }

	/** World position of a cell center, at the sampled ground height (or grid origin Z if none). */
	FVector CellToWorld(const FIntPoint& Cell) const;

	/** Cell containing a world position. May be out of bounds, so check IsValidCell. */
	FIntPoint WorldToCell(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsWalkable(const FVector& WorldLocation) const;

	uint8 GetCost(const FIntPoint& Cell) const;

	const TArray<uint8>& GetCostField() const { return CostField; }
	const TArray<float>& GetCellHeights() const { return CellHeights; }
	
	// ---- Flow field (delegates to FFlowFieldSolver) ----

	/** Single goal by world position. Solves immediately. Returns false if the goal is blocked or out of bounds. */
	UFUNCTION(BlueprintCallable, Category = "Grid|Flow")
	bool SetGoal(const FVector& GoalWorldLocation);

	/** Multiple goals. Agents head to whichever is nearest. */
	bool SetGoals(const TArray<FIntPoint>& GoalCells);

	/** Normalized direction (Z = 0) an agent at this location should move. Zero if blocked, at goal or unreachable. */
	UFUNCTION(BlueprintPure, Category = "Grid|Flow")
	FVector GetFlowDirection(const FVector& WorldLocation) const;

	/** Draws an arrow per cell showing the flow direction. */
	UFUNCTION(BlueprintCallable, Category = "Grid|Flow")
	void DrawDebugFlowField(float Duration = 10.f) const;

	/** Direct access to the solver and its results. */
	const FFlowFieldSolver& GetSolver() const { return Solver; }
	
protected:
	virtual void BeginPlay() override;

private:
	/** Per-cell cost. CostBlocked = impassable. Row-major: index = Y * Width + X. */
	UPROPERTY(Transient)
	TArray<uint8> CostField;

	/** Ground Z per cell. */
	UPROPERTY(Transient)
	TArray<float> CellHeights;
	
	/** Runs the BFS. Holds the integration and flow fields. */
	FFlowFieldSolver Solver;

	/** World position of the grid's minimum corner (cell 0,0's corner). */
	FVector GridOrigin = FVector::ZeroVector;

};
