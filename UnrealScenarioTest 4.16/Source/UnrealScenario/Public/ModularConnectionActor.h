// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "EditableConnectionActor.h"
#include "ModularConnectionActor.generated.h"

UCLASS(ClassGroup = UnrealScenario)
/**
 * Actor made by a set of identical objects (modules), arranged in a specified (editable) structure.
 */
class UNREALSCENARIO_API AModularConnectionActor : public AEditableConnectionActor
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
        Template blueprint used as corner module.
        */
		TSubclassOf<AActor> CornerModuleBlueprint;

	UPROPERTY(VisibleAnywhere, AdvancedDisplay, BlueprintReadOnly, Category = "Unreal Scenario")
		/**
        Corner modules between sections.
        */
		TArray<AActor*> CornerModules;

	// Sets default values for this actor's properties
	AModularConnectionActor();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:

	// Flags that tell for each section if it is a corner section.
	TArray<bool> CornerFlags;



	virtual bool CheckSectionTemplate() override;

	virtual void UpdateSectionsImpl() override;

	virtual void DestroySectionsImpl() override;

	virtual void OnEditConfirm();

	// Reuse or add a corner actor at the given index.
	AActor* GetOrAddCorner(int32 Index);

	// Destroy corner actors starting from the given index.
	void RemoveCornersFrom(int32 Index);
};
