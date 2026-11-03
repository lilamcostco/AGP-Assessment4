// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlowFieldGrid.h"
#include "AGP/Characters/EnemyCharacter.h"
#include "AGP/Characters/PlayerCharacter.h"
#include "GameFramework/Actor.h"
#include "FlowFieldManager.generated.h"

UCLASS()
class AGP_API AFlowFieldManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlowFieldManager();
	
	UPROPERTY(EditAnywhere, Category = "Flow")
	TSubclassOf<AFlowFieldGrid> GridClass;
	
	UFUNCTION(BlueprintPure, Category = "Flow")
	AFlowFieldGrid* GetGrid() const { return FlowGrid; }
	
	UFUNCTION(Category = "Flow")
	void SetGoalLocation();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// How often to check for updating the target
	UPROPERTY(EditAnywhere, Category = "Flow")
	float GoalUpdateInterval = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Flow|Debug")
	bool bDrawFlowDebug = true;

	FTimerHandle GoalTimer;
	TArray<FIntPoint> LastGoalCells;
	
private:
	UPROPERTY()
	TObjectPtr<AFlowFieldGrid> FlowGrid;
	
	TArray<AEnemyCharacter*> Enemies;
	void SetEnemies();
	
};
