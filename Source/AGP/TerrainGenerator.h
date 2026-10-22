// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainGenerator.generated.h"

class UProceduralMeshComponent;

UCLASS()
class AGP_API ATerrainGenerator : public AActor
{
    GENERATED_BODY()

public:
    ATerrainGenerator();

protected:
    virtual void BeginPlay() override;

private:
    void GenerateTerrain();

    UPROPERTY()
    UProceduralMeshComponent* Mesh;
};