// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "ElementDataComponent.h"
#include "ConnectionElementComponent.generated.h"

UCLASS(ClassGroup = UnrealScenario)
/**
Component holding data for a connection element.
A connection element connects other elements through their sockets, following a defined path.
It is implemented using AEditableConnectionActor and its derived classes.
 */
class UNREALSCENARIO_API UConnectionElementComponent : public UElementDataComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		List of connection anchors, used to store connection path data.
		*/
		TArray<FConnectionAnchor> ConnectionPath;

	// Sets default values for this component's properties
	UConnectionElementComponent();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
		Apply the currently defined connection path to the owner actor.
		*/
		virtual void ExportConnectionPath();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
		Get the connection path from the owner actor.
		*/
		virtual void ImportConnectionPath();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the connection path from the owner actor.
		*/
		float ComputeConnectionPathLength();
};
