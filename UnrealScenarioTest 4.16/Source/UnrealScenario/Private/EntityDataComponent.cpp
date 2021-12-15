// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "EntityDataComponent.h"


// Sets default values for this component's properties
UEntityDataComponent::UEntityDataComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UEntityDataComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}
