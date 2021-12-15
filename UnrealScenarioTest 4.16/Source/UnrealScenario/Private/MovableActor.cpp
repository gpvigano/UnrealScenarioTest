// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "MovableActor.h"


// Sets default values
AMovableActor::AMovableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	DefaultSceneRoot->CreationMethod = EComponentCreationMethod::Native;
	DefaultSceneRoot->SetRelativeLocation(FVector::ZeroVector);
	DefaultSceneRoot->SetRelativeRotation(FRotator::ZeroRotator);

	RootComponent = DefaultSceneRoot;

}

// Called when the game starts or when spawned
void AMovableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMovableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

