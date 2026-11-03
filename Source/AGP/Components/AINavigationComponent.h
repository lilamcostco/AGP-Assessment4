// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AGP/FlowField/FlowFieldGrid.h"
#include "Components/ActorComponent.h"
#include "AINavigationComponent.generated.h"

UENUM(BlueprintType)
enum class EAIState : uint8
{
	Idle,
	Chasing
};


class AFlowFieldManager;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AGP_API UAINavigationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAINavigationComponent();

	UFUNCTION( BlueprintCallable )
	void UpdateFlowField(AFlowFieldGrid* NewFlowField);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	TObjectPtr<AFlowFieldManager> Manager;
	UPROPERTY()
	TObjectPtr<AFlowFieldGrid> FlowField;
	
	FIntPoint CurrentCell;
	FVector MoveDirection;
	
	AFlowFieldGrid* GetGrid();
	void MoveEnemy(float DeltaTime);
	
	
	// void CheckFlowFieldCell();
	// updates only when the active cell has changed
	void UpdateMoveDirection();
};
