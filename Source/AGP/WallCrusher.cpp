#include "WallCrusher.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AWallCrusher::AWallCrusher()
{
    PrimaryActorTick.bCanEverTick = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

    NorthWall = CreateDefaultSubobject<UStaticMeshComponent>("NorthWall");
    NorthWall->SetupAttachment(RootComponent);

    SouthWall = CreateDefaultSubobject<UStaticMeshComponent>("SouthWall");
    SouthWall->SetupAttachment(RootComponent);

    EastWall = CreateDefaultSubobject<UStaticMeshComponent>("EastWall");
    EastWall->SetupAttachment(RootComponent);

    WestWall = CreateDefaultSubobject<UStaticMeshComponent>("WestWall");
    WestWall->SetupAttachment(RootComponent);

    // Load the default cube mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh>
        CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

    if (CubeMesh.Succeeded())
    {
        NorthWall->SetStaticMesh(CubeMesh.Object);
        SouthWall->SetStaticMesh(CubeMesh.Object);
        EastWall->SetStaticMesh(CubeMesh.Object);
        WestWall->SetStaticMesh(CubeMesh.Object);
    }

    // Make them wall-shaped
    NorthWall->SetWorldScale3D(FVector(1, 100, 40));
    SouthWall->SetWorldScale3D(FVector(1, 100, 40));

    EastWall->SetWorldScale3D(FVector(100, 1, 40));
    WestWall->SetWorldScale3D(FVector(100, 1, 40));
}

void AWallCrusher::BeginPlay()
{
    Super::BeginPlay();

    NorthWall->SetWorldLocation(
        CentrePoint + FVector(0.f, SpawnDistance, 0.f));

    SouthWall->SetWorldLocation(
        CentrePoint - FVector(0.f, SpawnDistance, 0.f));

    EastWall->SetWorldLocation(
        CentrePoint + FVector(SpawnDistance, 0.f, 0.f));

    WestWall->SetWorldLocation(
        CentrePoint - FVector(SpawnDistance, 0.f, 0.f));
}

void AWallCrusher::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    MoveWall(NorthWall, CentrePoint, DeltaTime);
    MoveWall(SouthWall, CentrePoint, DeltaTime);
    MoveWall(EastWall, CentrePoint, DeltaTime);
    MoveWall(WestWall, CentrePoint, DeltaTime);
}

void AWallCrusher::MoveWall(
    UStaticMeshComponent* Wall,
    const FVector& Target,
    float DeltaTime)
{
    FVector NewLocation =
        FMath::VInterpConstantTo(
            Wall->GetComponentLocation(),
            Target,
            DeltaTime,
            MoveSpeed);

    Wall->SetWorldLocation(NewLocation);
}
