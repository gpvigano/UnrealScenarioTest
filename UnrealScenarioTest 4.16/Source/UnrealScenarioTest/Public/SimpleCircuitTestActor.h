// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "UnrealScenarioTestActor.h"
#include "SimpleCircuitTestActor.generated.h"

UCLASS(ClassGroup = UnrealScenarioTest)
/**
* Unreal Scenario test for SimplECircuit digital system.
*/
class UNREALSCENARIOTEST_API ASimpleCircuitTestActor : public AUnrealScenarioTestActor
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Unreal Scenario|Circuit Test")
		/**
		Identifier (in AssetIndexComponent) of the actor used to simulate cables.
		*/
		FString CableTemplateName = TEXT("CircuitCableTemplate");

	ASimpleCircuitTestActor();

	/**
	Update visual hints about suggested/forbidden actions.
	*/
	virtual void UpdateHints() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:

	UPROPERTY(VisibleAnywhere, AdvancedDisplay, BlueprintReadOnly, Category = "Unreal Scenario|Circuit Test")
		/**
		Cable actors used to simulate circuit components connections.
		*/
		TArray<AActor*> Cables;

	UPROPERTY()
		AActor* NewCableActor = nullptr;

	UPROPERTY()
		UPrimitiveComponent* StartLeadComp = nullptr;

	// Cables that must be updated because connected to a moving component
	TArray<AActor*> MovingCables;

	// Number of cables currently used
	int NumCables = 0;

	// Used to keep track of the distance of the end point of a new cable
	float NewCableEndDistance = 0.0f;

	// Tag used to identify component leads as connected
	static FName ConnectedLeadTag;


	virtual void BeginPlay() override;

	virtual void OnEntityClicked(AActor* ClickedEntity, UPrimitiveComponent* ClickedComp) override;

	virtual void OnStartMovingEntity(AActor* MovedActor) override;

	virtual void OnEntityMoved(AActor* MovedActor) override;

	virtual void OnStopMovingEntity(AActor* MovedActor) override;

	UFUNCTION()
		virtual void OnBeginCursorOverEntity(UPrimitiveComponent* TouchedComponent) override;

	UFUNCTION()
		virtual void OnEndCursorOverEntity(UPrimitiveComponent* TouchedComponent) override;

	virtual bool IsEditing() override;

	virtual void UpdateEditing(float DeltaTime) override;

	virtual void CancelEditing() override;

	virtual void SyncEntityState(
		const std::shared_ptr<discenfw::Entity> ScenEntity,
		const std::shared_ptr<discenfw::xp::EntityState> XpEntityState,
		UEntityStateComponent* compState) override;

	virtual void SyncRelationships(
		const std::shared_ptr<discenfw::xp::EnvironmentState> XpScenarioState) override;


	// Get the cable with the given index or create it if it does not exist.
	AActor* GetOrCreateCable(int CableIndex = -1);

	// Cancel any wiring operation in progress, remove the cable being connected, if any.
	void StopWiring();

	// Activate the given switch to toggle its state.
	void SwitchSwitch(AActor* ClickedEntity);

	// Connect the current component lead to the selected component lead.
	void ConnectLead(AActor* ClickedEntity, UPrimitiveComponent* ClickedComp);

	// Disonnect the selected component lead from any other component lead.
	void DisconnectLead(AActor* ClickedEntity, UPrimitiveComponent* ClickedComp);

	// Update visual hints according to the given actions set.
	// Call UpdateActionHint() for each action.
	void UpdateActionsHints(
		const std::vector<discenfw::xp::Action>& ActionList,
		UMaterialInterface* IconMaterial,
		UEntityDataComponent* StartEntity,
		bool IncludeConnections = true);

	// Update visual hints according to the given action.
	// If a wiring is in progress (StartEntity not null) proper hints are displayed.
	void UpdateActionHint(
		const discenfw::xp::Action& WhichAction,
		UMaterialInterface* IconMaterial,
		UEntityDataComponent* StartEntity,
		bool IncludeConnections = true);
};

