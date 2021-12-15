// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "EntityActor.h"


// Sets default values
AEntityActor::AEntityActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	EntityData = CreateDefaultSubobject<UEntityDataComponent>(TEXT("EntityData"));
	EntityData->CreationMethod = EComponentCreationMethod::Native;
}


#if WITH_EDITOR
void AEntityActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UEntityDataComponent* entityComponent = static_cast<UEntityDataComponent*>(GetComponentByClass(UEntityDataComponent::StaticClass()));
	if (entityComponent->Identifier.IsEmpty())
	{
		entityComponent->Identifier = GetActorLabel();
	}
}
#endif

