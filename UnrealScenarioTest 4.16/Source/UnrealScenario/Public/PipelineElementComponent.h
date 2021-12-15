// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "ConnectionElementComponent.h"
#include "PipelineActor.h"



#include "PipelineElementComponent.generated.h"

UCLASS(ClassGroup = UnrealScenario, meta = (BlueprintSpawnableComponent))
/**
 * Component in charge to exchange data between the scenario and the virtual pipeline object.
 */
class UNREALSCENARIO_API UPipelineElementComponent : public UConnectionElementComponent
{
	GENERATED_BODY()
	
public:

		/**
        Apply the currently defined connection path to the owner actor.
        */
		virtual void ExportConnectionPath() override;

	
		/**
        Get the connection path from the owner actor.
        */
		virtual void ImportConnectionPath() override;
	
protected:

	// Reference to the pipeline (owner) actor.
	APipelineActor* PipelineActor = nullptr;


	// Get the reference to the pipeline (owner) actor.
	APipelineActor* GetPipelineActor();

};
