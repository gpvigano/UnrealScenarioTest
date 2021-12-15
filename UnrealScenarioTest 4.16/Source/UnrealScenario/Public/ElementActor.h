// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "MovableActor.h"
#include "ElementDataComponent.h"

#include "ElementActor.generated.h"

UCLASS(Blueprintable, ClassGroup = UnrealScenario)
/**
* Unreal Scenario generic element actor, used only when no related asset is available.
*/
class UNREALSCENARIO_API AElementActor : public AMovableActor
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		UElementDataComponent* ElementData;


	// Sets default values for this actor's properties
	AElementActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
};
