// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WallCrusher.generated.h"

UCLASS()
class AGP_API AWallCrusher : public AActor
{
    GENERATED_BODY()

public:
    AWallCrusher();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY()
    UStaticMeshComponent* NorthWall;

    UPROPERTY()
    UStaticMeshComponent* SouthWall;

    UPROPERTY()
    UStaticMeshComponent* EastWall;

    UPROPERTY()
    UStaticMeshComponent* WestWall;

    void MoveWall(
        UStaticMeshComponent* Wall,
        const FVector& Target,
        float DeltaTime);

    FVector CentrePoint = FVector(1270.f, 3890.f, 0.f);

    UPROPERTY(EditAnywhere)
    float MoveSpeed = 50.f;

    UPROPERTY(EditAnywhere)
    float SpawnDistance = 5000.f;
};