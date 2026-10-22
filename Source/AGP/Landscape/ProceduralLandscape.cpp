// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralLandscape.h"

#include "KismetProceduralMeshLibrary.h"
#include "ProceduralMeshComponent.h"
#include "AGP/Pathfinding/PathfindingSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AProceduralLandscape::AProceduralLandscape()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Procedural Mesh"));
	SetRootComponent(ProceduralMesh);
	
	Width = 10;
	Height = 10;
	VertexSpacing = 1000.0f;
	
	PerlinScale = 1000.0f;
	PerlinRoughness = 0.00012f;

}

// Called when the game starts or when spawned
void AProceduralLandscape::BeginPlay()
{
	Super::BeginPlay();
}

void AProceduralLandscape::CreateSimplePlane()
{
	constexpr float VertexMaxHeight = 500.0f;
	
	Vertices.Append({
		FVector(0.0f, 0.0f, FMath::FRandRange(-VertexMaxHeight, VertexMaxHeight)),
		FVector(VertexSpacing, 0.0f, FMath::FRandRange(-VertexMaxHeight, VertexMaxHeight)),
		FVector(0.0f, VertexSpacing, FMath::FRandRange(-VertexMaxHeight, VertexMaxHeight)),
		FVector(VertexSpacing, VertexSpacing, FMath::FRandRange(-VertexMaxHeight, VertexMaxHeight))
	});

	for (FVector Vertex : Vertices)
	{
		DrawDebugSphere(GetWorld(), Vertex, 50.0f, 4, FColor::Blue, true);
	}
	
	Triangles.Append({0,2,1,1,2,3});
	UVCoords.Append({FVector2D::Zero(),FVector2D::UnitX(),FVector2D::UnitY(),FVector2D::UnitVector});
	
	if (ProceduralMesh)
	{
		ProceduralMesh->CreateMeshSection(0,Vertices, Triangles, TArray<FVector>(), UVCoords,TArray<FColor>(), TArray<FProcMeshTangent>(), true);
	}
}

// Called every frame
void AProceduralLandscape::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bShouldRegenerate)
	{
		ClearLandscape();
		GenerateLandscape();
		bShouldRegenerate = false;
	}

}

void AProceduralLandscape::GenerateLandscape()
{
	PerlinOffset = FMath::FRandRange(-1'000'000.0f, 1'000'000.0f);
	
	for (int j = 0; j < Height; ++j)
	{
		for (int i = 0; i < Width; ++i)
		{
			FVector VertexPosition = FVector(i*VertexSpacing, j*VertexSpacing, 0.0f);
			VertexPosition.Z = PerlinScale * FMath::PerlinNoise2D(PerlinRoughness * FVector2D(VertexPosition.X, VertexPosition.Y) + PerlinOffset);
			
			// store Vertices and UV-Coords.
			Vertices.Add(VertexPosition);
			UVCoords.Add(FVector2D(i,j));
			// DrawDebugSphere(GetWorld(), VertexPosition, 50.0f, 4, FColor::Blue, true);
			
			// ignore the left and top edges when making triangles.
			if (i==Width-1) continue;
			if (j==Height-1) continue;

			Triangles.Append({
			j*Width + i,
			(j+1)*Width + i,
			j*Width + (i+1),
			j*Width + (i+1),
			(j+1)*Width + i,
			(j+1)*Width + (i+1),
			});
			
			
		}
	}
	
	// Create the mesh using the above vertices, triangles, and UV-Coords.
	if (ProceduralMesh)
	{
		TArray<FVector> Normals;
		TArray<FProcMeshTangent> Tangents;
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UVCoords,Normals, Tangents);
		ProceduralMesh->CreateMeshSection(0,Vertices, Triangles, Normals, UVCoords,TArray<FColor>(), Tangents, true);
	}
	
	// Place NavigationNodes at vertices via PathFindingSubsystem.
	if (UPathfindingSubsystem* PathfindingSubsystem = GetWorld()->GetSubsystem<UPathfindingSubsystem>())
	{
		PathfindingSubsystem->PlaceProceduralNodes(Vertices, Width, Height, this);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No pathfinding subsystem is available"));
	}
}

void AProceduralLandscape::ClearLandscape()
{
	Vertices.Empty();
	Triangles.Empty();
	UVCoords.Empty();
	
	if (ProceduralMesh)
	{
		ProceduralMesh->ClearMeshSection(0);
	}
	UKismetSystemLibrary::FlushPersistentDebugLines(GetWorld());
}

bool AProceduralLandscape::ShouldTickIfViewportsOnly() const
{
	return true;
}

