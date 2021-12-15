// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "MovableActor.h"
#include "EntityDataComponent.h"


#include "EntityActor.generated.h"

UCLASS(Blueprintable, ClassGroup = UnrealScenario)
/**
* Unreal Scenario generic entity actor, used only when no related asset is available.
*/
class UNREALSCENARIO_API AEntityActor : public AMovableActor
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		UEntityDataComponent* EntityData;

	// Sets default values for this actor's properties
	AEntityActor();


#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

};
