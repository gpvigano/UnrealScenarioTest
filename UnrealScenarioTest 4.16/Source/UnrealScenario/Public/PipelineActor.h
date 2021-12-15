// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Engine/SplineMeshActor.h"

#include "EditableConnectionActor.h"
#include "UnrealScenario.h"

#include "PipelineActor.generated.h"

UCLASS(ClassGroup = UnrealScenario)
/**
* Unreal Scenario actor for editable pipelines.
*/
class UNREALSCENARIO_API APipelineActor : public AEditableConnectionActor
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
        Smoothing around the corners.
        */
		float CurveSmoothing;

	// Sets default values for this actor's properties
	APipelineActor();


	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:

	virtual bool CheckSectionTemplate() override;

	virtual void UpdateSectionsImpl() override;

	void CalculateTangents();

	void CalculateUpDirs();
};
