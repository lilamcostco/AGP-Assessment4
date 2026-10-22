// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralLandscape.generated.h"

class UPathfindingSubsystem;

UCLASS()
class AGP_API AProceduralLandscape : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProceduralLandscape();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere)
	class UProceduralMeshComponent* ProceduralMesh;
	
	UPROPERTY()
	TArray<FVector> Vertices;
	
	UPROPERTY()
	TArray<int32> Triangles;
	
	UPROPERTY()
	TArray<FVector2D> UVCoords;
	
	UPROPERTY(EditAnywhere)
	bool bShouldRegenerate;
	
	UPROPERTY(EditAnywhere)
	int32 Width;
	
	UPROPERTY(EditAnywhere)
	int32 Height;
	
	UPROPERTY(EditAnywhere)
	float VertexSpacing;
	
	UPROPERTY(EditAnywhere)
	float PerlinScale;
	
	UPROPERTY(EditAnywhere)
	float PerlinRoughness;
	
	UPROPERTY(EditAnywhere)
	float PerlinOffset;
	
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void CreateSimplePlane();
	
	void GenerateLandscape();
	
	void ClearLandscape();
	
	bool ShouldTickIfViewportsOnly() const override;

};
