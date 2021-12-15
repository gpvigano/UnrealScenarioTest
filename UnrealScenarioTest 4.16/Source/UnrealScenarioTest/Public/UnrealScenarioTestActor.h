// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "UnrealScenarioUIActor.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "UnrealScenario/Public/CatalogItemWidget.h"

class UTextRenderComponent;
class UTextBlock;


#include "UnrealScenarioTestActor.generated.h"

UCLASS(ClassGroup = UnrealScenarioTest)
/**
 * Actor used to build tests with Unreal Scenario module.
 */
class UNREALSCENARIOTEST_API AUnrealScenarioTestActor : public AUnrealScenarioUIActor
{
	GENERATED_BODY()

public:

	AUnrealScenarioTestActor();


	/**
	Move an element to the next element slot location
	*/
	virtual void MoveActorToNextSlotLocation(AActor* ElementActor) override;

	/**
	Remove all the scene actors related to the scenario entities, reset the spawn location.
	*/
	virtual void ClearScene() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif


protected:

	UPROPERTY()
		// location of the first element spawned.
		FVector InitElementSpawnSlot;

	UPROPERTY()
		// List of text render components found in TooltipLabel
		TArray<UTextRenderComponent*> LabelTextRenderers;

	UPROPERTY()
		// Text block found in TooltipLabel
		UTextBlock* LabelTextBlock = nullptr;


	virtual void BeginPlay() override;

	virtual void OnCancel() override;

	// Set the given text to all text render components of TooltipLabel, update its location
	void SetLabelText(const FString& LabelText, const FVector& LabelLocation);
};
