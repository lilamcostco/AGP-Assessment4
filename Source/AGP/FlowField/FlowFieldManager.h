// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FlowFieldGrid.h"
#include "GameFramework/Actor.h"
#include "FlowFieldManager.generated.h"

class UAINavigationComponent;

UCLASS()
class AGP_API AFlowFieldManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlowFieldManager();
	
	UFUNCTION(BlueprintPure, Category = "Flow")
	AFlowFieldGrid* GetGrid() const { return FlowGrid; }
	
	UFUNCTION(BlueprintCallable, Category = "Flow")
	void SetGoalLocation();
	
	void RegisterEnemy(UAINavigationComponent* Enemy);
	void UnregisterEnemy(UAINavigationComponent* Enemy);

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
	
	TArray<TWeakObjectPtr<UAINavigationComponent>> Enemies;
	FTimerHandle StateTimer;
	void UpdateEnemyStates();
	
	UPROPERTY(EditAnywhere, Category = "Flow")
	float StateUpdateInterval = 0.25f;
};
