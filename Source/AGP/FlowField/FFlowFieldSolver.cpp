// Fill out your copyright notice in the Description page of Project Settings.


#include "FFlowFieldSolver.h"

namespace
{
	// 8-way neighbors. The first 4 are orthogonal, the last 4 are diagonal.
	const FIntPoint NeighborOffsets[8] =
	{
		FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1),
		FIntPoint(1, 1), FIntPoint(1, -1), FIntPoint(-1, 1), FIntPoint(-1, -1)
	};

	// Integer step costs (10 and ~10*sqrt2) so everything stays in uint16.
	constexpr uint32 StraightStep = 10;
	constexpr uint32 DiagonalStep = 14;
}

bool FFlowFieldSolver::Solve(const FIntPoint& InDimensions, const TArray<uint8>& CostField, const TArray<FIntPoint>& GoalCells)
{
	Reset(); // preliminary reset
	
	if (InDimensions.X <= 0 || InDimensions.Y <= 0 || CostField.Num() != InDimensions.X * InDimensions.Y)
	{
		return false;
	}
	
	Dimensions = InDimensions;
	
	bool bHasValidGoal = false;
	for (const FIntPoint& Goal : GoalCells)
	{
		if (GetCost(CostField, Goal) != CostBlocked)
		{
			bHasValidGoal = true;
			break;
		}
	}
	if (!bHasValidGoal)
	{
		Reset();
		return false;
	}
	
	BuildIntegrationField(CostField, GoalCells);
	BuildFlowField(CostField);
	return true;
}

void FFlowFieldSolver::Reset()
{
	Dimensions = FIntPoint::ZeroValue;
	IntegrationField.Reset();
	FlowField.Reset();
}

bool FFlowFieldSolver::IsValidCell(const FIntPoint& Cell) const
{
	return Cell.X >= 0 && Cell.Y >= 0 && Cell.X < Dimensions.X && Cell.Y < Dimensions.Y;
}

FVector2D FFlowFieldSolver::GetDirection(const FIntPoint& Cell) const
{
	if (!IsValidCell(Cell) || !FlowField.IsValidIndex(CellToIndex(Cell)))
	{
		return FVector2D::ZeroVector;
	}
	return FlowField[CellToIndex(Cell)];
}

uint16 FFlowFieldSolver::GetIntegration(const FIntPoint& Cell) const
{
	if (!IsValidCell(Cell) || !FlowField.IsValidIndex(CellToIndex(Cell)))
	{
		return Unreachable;
	}
	return IntegrationField[CellToIndex(Cell)];
}

uint8 FFlowFieldSolver::GetCost(const TArray<uint8>& CostField, const FIntPoint& Cell) const
{
	if (!IsValidCell(Cell) || !CostField.IsValidIndex(CellToIndex(Cell)))
	{
		return CostBlocked;
	}
	return CostField[CellToIndex(Cell)];
}


bool FFlowFieldSolver::CanStep(const TArray<uint8>& CostField, const FIntPoint& Cell, const FIntPoint& Offset) const
{
	if (GetCost(CostField, Cell + Offset) == CostBlocked)
	{
		return false;
	}

	// Diagonal moves can't cut the corner of a blocked cell.
	if (Offset.X != 0 && Offset.Y != 0)
	{
		if (GetCost(CostField, FIntPoint(Cell.X + Offset.X, Cell.Y)) == CostBlocked ||
			GetCost(CostField, FIntPoint(Cell.X, Cell.Y + Offset.Y)) == CostBlocked)
		{
			return false;
		}
	}
	return true;
}

void FFlowFieldSolver::BuildIntegrationField(const TArray<uint8>& CostField, const TArray<FIntPoint>& GoalCells)
{
	const int32 NumCells = Dimensions.X * Dimensions.Y;
	IntegrationField.Init(Unreachable, NumCells);

	// Array-backed FIFO queue. Head advances instead of popping from the front.
	TArray<int32> Queue;
	Queue.Reserve(NumCells);
	int32 Head = 0;

	// Seed every goal at cost 0.
	for (const FIntPoint& Goal : GoalCells)
	{
		if (GetCost(CostField, Goal) == CostBlocked)
		{
			continue;
		}
		const int32 Index = CellToIndex(Goal);
		IntegrationField[Index] = 0;
		Queue.Add(Index);
	}

	while (Head < Queue.Num())
	{
		const int32 CurrentIndex = Queue[Head++];
		const FIntPoint Current = IndexToCell(CurrentIndex);
		const uint32 CurrentValue = IntegrationField[CurrentIndex];

		for (int32 i = 0; i < 8; ++i)
		{
			const FIntPoint& Offset = NeighborOffsets[i];
			if (!CanStep(CostField, Current, Offset))
			{
				continue;
			}

			const int32 NextIndex = CellToIndex(Current + Offset);

			// Cost to enter the neighbor, scaled by its terrain cost.
			const uint32 Step = (i < 4 ? StraightStep : DiagonalStep) * CostField[NextIndex];
			const uint32 NewValue = CurrentValue + Step;

			if (NewValue >= Unreachable)
			{
				continue; // Would overflow uint16, so treat as unreachable
			}

			// Relaxation: re-queue a cell if a cheaper route is found. With uniform
			// terrain cost this is plain BFS, and with varying cost it still converges.
			if (NewValue < IntegrationField[NextIndex])
			{
				IntegrationField[NextIndex] = static_cast<uint16>(NewValue);
				Queue.Add(NextIndex);
			}
		}
	}
}

void FFlowFieldSolver::BuildFlowField(const TArray<uint8>& CostField)
{
	const int32 NumCells = Dimensions.X * Dimensions.Y;
	FlowField.Init(FVector2D::ZeroVector, NumCells);

	for (int32 Index = 0; Index < NumCells; ++Index)
	{
		const uint16 Value = IntegrationField[Index];
		if (Value == Unreachable || Value == 0)
		{
			continue; // Blocked, unreachable, or already at a goal
		}

		const FIntPoint Cell = IndexToCell(Index);

		uint16 BestValue = Value;
		FIntPoint BestOffset(0, 0);

		for (const FIntPoint& Offset : NeighborOffsets)
		{
			if (!CanStep(CostField, Cell, Offset))
			{
				continue;
			}

			const uint16 NeighborValue = IntegrationField[CellToIndex(Cell + Offset)];
			if (NeighborValue < BestValue)
			{
				BestValue = NeighborValue;
				BestOffset = Offset;
			}
		}

		if (BestOffset != FIntPoint(0, 0))
		{
			FlowField[Index] = FVector2D(BestOffset.X, BestOffset.Y).GetSafeNormal();
		}
	}
}