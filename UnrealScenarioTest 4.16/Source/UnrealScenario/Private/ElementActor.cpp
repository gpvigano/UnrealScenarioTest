// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "ElementActor.h"
#include "ElementDataComponent.h"


// Sets default values
AElementActor::AElementActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ElementData = CreateDefaultSubobject<UElementDataComponent>(TEXT("ElementData"));
	ElementData->CreationMethod = EComponentCreationMethod::Native;

	// TODO: move this in a new AMutableElementActor
	//RepresentationComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RepresentationComponent"));
	//RepresentationComponent->SetupAttachment(RootComponent);
	//AddOwnedComponent(RepresentationComponent);

	// ...and in AMutableElementActor header file:
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
	// /**
	// Representation of the element, set primary the static mesh and optionally its transformation.
	// */
	//	UStaticMeshComponent* RepresentationComponent;
}

// Called when the game starts or when spawned
void AElementActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AElementActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

