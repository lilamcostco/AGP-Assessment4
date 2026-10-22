// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
struct FFlowFieldSolver
{
	static constexpr uint8 CostBlocked = 255;
	
	static constexpr uint16 Unreachable = MAX_uint16;
	
	
	/**
	 * Runs a BFS from the goal then builds the flow field
	 */ 
	bool Solve(const FIntPoint& InDimensions, const TArray<uint8>& CostField, const TArray<FIntPoint>& GoalCell);
	
	void Reset();
	
	bool HasData() const { return IntegrationField.Num() > 0 && IntegrationField.Num() == FlowField.Num(); }

	bool IsValidCell(const FIntPoint& Cell) const;
	
	FVector2D GetDirection(const FIntPoint& Cell) const;
	
	uint16 GetIntegration(const FIntPoint& Cell) const;
	
	const FIntPoint& GetDimensions() const { return Dimensions; }
	const TArray<uint16>& GetIntegrationField() const { return IntegrationField; }
	const TArray<FVector2D>& GetFlowField() const { return FlowField; }
	
private:
	void BuildIntegrationField(const TArray<uint8>& CostField, const TArray<FIntPoint>& GoalCells);
	void BuildFlowField(const TArray<uint8>& CostField);
	
	// True if stepping from Cell by Offset is legal 
	bool CanStep(const TArray<uint8>& CostField, const FIntPoint& Cell, const FIntPoint& Offset) const;
	
	uint8 GetCost(const TArray<uint8>& CostField, const FIntPoint& Cell) const;
	int32 CellToIndex(const FIntPoint& Cell) const { return Cell.Y * Dimensions.X + Cell.X; }
	FIntPoint IndexToCell(int32 Index) const { return FIntPoint(Index % Dimensions.X, Index / Dimensions.X); }
	
	FIntPoint Dimensions = FIntPoint::ZeroValue;
	
	// Cost-to-goal per cell
	TArray<uint16> IntegrationField;
	
	// Unit Direction per cell toward the goal
	TArray<FVector2D> FlowField;
};
