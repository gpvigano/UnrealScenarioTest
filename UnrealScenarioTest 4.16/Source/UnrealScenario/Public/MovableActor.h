// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovableActor.generated.h"

UCLASS(ClassGroup = UnrealScenario)
/**
* Basic movable actor with a root scene component defined.
*/
class UNREALSCENARIO_API AMovableActor : public AActor
{
	GENERATED_BODY()
	
public:	

	UPROPERTY(VisibleAnywhere, AdvancedDisplay)
	USceneComponent* DefaultSceneRoot;

	// Sets default values for this actor's properties
	AMovableActor();
    
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
