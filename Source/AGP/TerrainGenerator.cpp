#include "TerrainGenerator.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"



ATerrainGenerator::ATerrainGenerator()
{
    UE_LOG(LogTemp, Warning, TEXT("TerrainGenerator Constructor"));

    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(
        TEXT("TerrainMesh")
    );

    RootComponent = Mesh;
}

void ATerrainGenerator::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("BeginPlay called"));

    GenerateTerrain();
}

void ATerrainGenerator::GenerateTerrain()
{
    if (!Mesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Mesh Invalid"));
        return;
    }

    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FColor> Colors;

    const int GridSize = 50;
    const float Spacing = 300.0f;

    float SeedX = FMath::RandRange(0.0f, 10000.0f);
    float SeedY = FMath::RandRange(0.0f, 10000.0f);

    for (int y = 0; y <= GridSize; y++)
    {
        for (int x = 0; x <= GridSize; x++)
        {
            float Height =
                FMath::PerlinNoise2D(
                    FVector2D(
                        SeedX + x * 0.1f,
                        SeedY + y * 0.1f
                    )
                ) * 300.0f;

            Vertices.Add(
                FVector(
                    x * Spacing,
                    y * Spacing,
                    Height
                )
            );

            Colors.Add(
                Height < 0
                ? FColor::Blue
                : FColor::Green
            );
        }
    }

    for (int y = 0; y < GridSize; y++)
    {
        for (int x = 0; x < GridSize; x++)
        {
            int Row1 = x + y * (GridSize + 1);
            int Row2 = Row1 + GridSize + 1;

            Triangles.Add(Row1);
            Triangles.Add(Row2);
            Triangles.Add(Row1 + 1);

            Triangles.Add(Row1 + 1);
            Triangles.Add(Row2);
            Triangles.Add(Row2 + 1);
        }
    }

    Mesh->CreateMeshSection(
        0,
        Vertices,
        Triangles,
        TArray<FVector>(),
        TArray<FVector2D>(),
        Colors,
        TArray<FProcMeshTangent>(),
        true
    );

    UE_LOG(LogTemp, Warning, TEXT("Terrain Created"));

    for (int32 i = 0; i < 30; i++)
{
    int32 RandomVertex = FMath::RandRange(
        0,
        Vertices.Num() - 1
    );

    FVector Location =
    GetActorTransform().TransformPosition(
        Vertices[RandomVertex]
    );

    Location.Z -= 500.0f;

    AStaticMeshActor* Rock =
        GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Location,
            FRotator(
                0.0f,
                FMath::RandRange(0.0f, 360.0f),
                0.0f
            )
        );

    if (Rock)
    {
        UStaticMeshComponent* MeshComponent =
            Rock->GetStaticMeshComponent();

        if (MeshComponent)
        {
            UStaticMesh* CubeMesh =
                LoadObject<UStaticMesh>(
                    nullptr,
                    TEXT("/Engine/BasicShapes/Cube.Cube")
                );

            if (CubeMesh)
            {
                UE_LOG(LogTemp, Warning, TEXT("Rock mesh loaded"));

                MeshComponent->SetStaticMesh(CubeMesh);

                UMaterialInterface* RockMaterial =
                    LoadObject<UMaterialInterface>(
                        nullptr,
                        TEXT("/Game/M_Rock.M_Rock")
                    );

                if (RockMaterial)
                {
                    MeshComponent->SetMaterial(
                        0,
                        RockMaterial
                    );
                };

                float ScaleX =
                    FMath::RandRange(2.0f, 3.0f);

                float ScaleY =
                    FMath::RandRange(2.0f, 3.0f);

                float ScaleZ =
                    FMath::RandRange(8.0f, 20.0f);

                Rock->SetActorScale3D(
                    FVector(
                        ScaleX,
                        ScaleY,
                        ScaleZ
                    )
                );
            }
        }
    }
}


}