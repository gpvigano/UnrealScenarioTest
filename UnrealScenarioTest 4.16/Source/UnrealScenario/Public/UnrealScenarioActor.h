// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/MeshComponent.h"

#include "AssetIndexComponent.h"
#include "ElementActor.h"
#include "UnrealScenario.h"
#include "CatalogComponent.h"
#include "Logger.h"
#include "ConnectionElementComponent.h"
#include "discenfw/DigitalScenarioFramework.h"
#include "discenfw/scen/ScenarioData.h"

#include "UnrealScenarioActor.generated.h"

UCLASS(ClassGroup = UnrealScenario)
/**
 * Actor implementing the scenario management for Unreal Scenario module.
 */
class UNREALSCENARIO_API AUnrealScenarioActor : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Path to the scenario file (JSON) relative to project or build root.
		*/
		FString ScenarioFileName = "scenario.json";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Load the scenario (if defined) at startup.
		*/
		bool bAutoLoadScenario = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Catalog items definition.
		*/
		UCatalogComponent* CatalogComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		If not empty Entity Type Actors are automatically collected from its children actors.
		*/
		UAssetIndexComponent* AssetIndexComponent;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Main scenario root actor.
		*/
		AActor* ScenarioActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Scale factor for units in scenario serialization.
		*/
		float UnitScale = 1.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario|Interaction")
		/**
		Enable overlapping among objects.
		*/
		bool bOverlappingEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario|Interaction")
		/**
		Use internal mouse interaction.
		*/
		bool bMouseInteractionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario|Interaction")
		/**
		Enable overlapping when dragging objects with the mouse.
		*/
		bool bOverlapOnDragging = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario|Interaction")
		/**
		Material used to highlight objects.
		*/
		UMaterialInterface* HighlightMaterial;


	UPROPERTY(EditInstanceOnly, AdvancedDisplay, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Print log messages on screen.
		*/
		bool bLogOnScreenEnabled = true;

	UPROPERTY(VisibleAnywhere, AdvancedDisplay, BlueprintReadOnly, Category = "Unreal Scenario")
		/**
		Actors built from entities.
		*/
		TArray<AActor*> EntityActors;

	UPROPERTY(EditInstanceOnly, AdvancedDisplay, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Set this to an entity actor to add it to the scenario.
		*/
		AActor* EntityActorToAdd = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario Simulation")
		/**
		Path of the scenario file (JSON) relative to project or build root.
		*/
		FString HistoryFileName = "";

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Unreal Scenario Simulation")
		/**
		Start the scenario simulation (if defined) at startup.
		*/
		bool bAutoStartSimulation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario Simulation", meta = (UIMin = -1.0, UIMax = 1.0))
		/**
		Speed multiplier for the simulation (negative values reverse the simulation progress).
		*/
		float SimulationSpeedMultiplier = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient, NonTransactional, Category = "Unreal Scenario Simulation", meta = (UIMin = 0, UIMax = 100.0))
		/**
		Simulation progress in percentage.
		*/
		float SimulationProgress = 0.0;


	// Sets default values for this actor's properties
	AUnrealScenarioActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the identifiers of the entities in the current scenario.
		*/
		TArray<FString> GetEntityIdentifiers();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Check if the current scenario has no entity defined.
		*/
		bool IsScenarioEmpty();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Count the entities in the scenario with the given category.
		*/
		int CountEntitiesByCategory(FString CategoryId);

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Add a new element to the current scenario
		*/
		AActor* AddElementFromCatalog(FString ItemId, bool bMoveToNextSlot = false);

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Duplicate the given element and add it to the current scenario.
		*/
		AActor* DuplicateElement(AActor* ElementActor);

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Delete an element from the current scenario.
		*/
		bool DeleteElement(AActor* ElementActor);

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Move an element to the next element slot location.
        Override this method in derived classes to place created actors
        to a specific location (default = ElementSpawnSlot).
		*/
		virtual void MoveActorToNextSlotLocation(AActor* ElementActor);

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Update the current scene creating actors from scenario data.
		*/
		void PopulateScene();

	UFUNCTION()
		/**
		Remove all the scene actors related to the scenario entities.
		*/
		virtual void ClearScene();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Update the current scenario data reading from scene actors and their components.
		*/
		void UpdateData();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
		Load the current scenario.
		*/
		void LoadScenarioData();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
		Save the current scenario.
		*/
		void SaveScenarioData();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
		Clear the current scenario.
		*/
		void ClearScenario();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Capture a screenshot from the viewport.
		*/
		void Screenshot();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Update the position of the dragged object (if any).
		*/
		void UpdatePositioning();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Cancel the positioning of the dragged object (if any), restoring its previous position.
		*/
		void CancelPositioning();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
		@todo To be implemented.
		*/
		void Undo();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the scenario data directory.
		*/
		FString ScenarioDir();


	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario Simulation")
		/**
		Play the simulation, if present in the current scenario.
		*/
		void PlaySimulation();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario Simulation")
		/**
		Pause the simulation, if playing in the current scenario.
		*/
		void PauseSimulation();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario Simulation")
		/**
		Stop (reset) the simulation, if playing in the current scenario.
		*/
		void StopSimulation();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:

	float MovingEntityDistance;

	FVector MovingEntityStartPos;
	FVector MovingEntityOffset;

	FRotator MovingEntityStartRot;

	// Optional tag used to mark primitives that cannot be highlighted
	FName DontHighlightTag;

	bool bUnrealScenarioInitialized = false;

	bool bUseCustomDepth = true;

	bool bTemporaryLockedSelection = false;

	//UPROPERTY()
	//	TArray<AConnectionElementActor*> ConnectionElementActors;

	UPROPERTY()
		TMap<UPrimitiveComponent*, FMaterialList> OrigMaterialMap;

	UPROPERTY()
		TMap<UPrimitiveComponent*, FMaterialList> SelectedOrigMaterialMap;

	UPROPERTY()
		TArray<UMaterialInterface*> OrigPrimMaterials;

	UPROPERTY()
		UPrimitiveComponent* HighlighedPrimComp = nullptr;

	UPROPERTY()
		UPrimitiveComponent* SocketPrimComp = nullptr;

	UPROPERTY()
		UPrimitiveComponent* OtherSocketPrimComp = nullptr;

	UPROPERTY()
		AActor* HighlighedEntityActor = nullptr;

	UPROPERTY()
		AActor* SelectedEntityActor = nullptr;

	UPROPERTY()
		AActor* MovingEntityActor = nullptr;

	UPROPERTY()
		FVector ElementSpawnSlot;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void InitDigitalScenarioFramework();

	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit);

	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//UFUNCTION()
	//	void OnActorOverlapBegin(AActor* ThisActor, AActor* OtherActor);

	//UFUNCTION()
	//	void OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	//UFUNCTION()
	//	void OnActorClicked(AActor* TouchedActor, FKey ButtonPressed);


	UFUNCTION()
		virtual void OnBeginCursorOverComponent(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
		virtual void OnEndCursorOverComponent(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
		void OnClickedComponent(UPrimitiveComponent* ClickedComp, FKey ButtonPressed);

	virtual void OnBeginCursorOverEntity(UPrimitiveComponent* TouchedComponent);

	virtual void OnEndCursorOverEntity(UPrimitiveComponent* TouchedComponent);

	virtual void OnEntityClicked(AActor* clickedEntity, UPrimitiveComponent* ClickedComp);

	virtual void OnStartMovingEntity(AActor* MovedActor) {}

	virtual void OnEntityMoved(AActor* MovedActor) {}

	virtual void OnStopMovingEntity(AActor* MovedActor) {}

	virtual void OnCancel();

	virtual void OnScenarioLoaded();

	virtual void OnEntityAdded();

	virtual void OnEntityRemoved();

	virtual bool IsEditing();

	virtual void UpdateEditing(float DeltaTime);

	virtual void UpdateSimulation(float DeltaTime);

	virtual void CancelEditing();

	virtual void SaveEntityPosition();

	virtual void DragSelectedEntity();

	//
	//	void SelectEntity(FName Identifier);

	AActor* CreateEntityFromActor(FString assetId, FString entityName, AActor* parentActor);

	AActor* CreateEntityFromAsset(FString entityType, FString entityName, AActor* parentActor);

	void HighlightActor(AActor* Actor, bool Highlighted);

	void HighlightComponent(UPrimitiveComponent* PrimitiveComponent, bool Highlighted);

	AActor* GetEntityFromActor(AActor* selectedActor);

	AActor* SelectEntityFromActor(AActor* selectedActor);

	AActor* SelectEntityActor(AActor* selectedActor);


	AActor* GetEntityActorFromId(const FString& entityId);

	AActor* GetEntityFromId(const FString& id);

	bool EntityActorExists(const FString& id);

	AActor* GetElementFromId(const FString& elemId);


	void SetActorSelectable(AActor* actor, bool includeAttachedActors = true);

	void UpdateEntityData(AActor* entityActor);

	void UpdateScenarioCamera();

	void GetViewPoint(FVector& pos, FVector& rot);


	/// @name DiScenFw connector
	///@{

	std::shared_ptr< discenfw::Entity > GetEntityByName(const FString& name);

	std::shared_ptr< discenfw::Element > GetElementByName(const FString& name);

	AActor* GetEntityActorFromId(const std::string& id);

	FVector ImportVector(const discenfw::Vector3D& vec);

	FTransform ImportCoordSysRot(const discenfw::CoordSys3D& coordSys);

	void SetElementActorTransform(AActor* targetActor, const discenfw::LocalTransform& localTransform);

	AActor* ImportEntity(const std::shared_ptr< const discenfw::Entity > entity, AActor* parentActor);

	void ImportElement(const std::shared_ptr< const discenfw::Element > elem, AActor* elemActor);

	//void ImportAggregate(const std::shared_ptr< discenfw::Aggregate > system, AAggregateActor* newActor);

	void ImportConnectionElement(
		const std::shared_ptr< const discenfw::ConnectionElement > coonectionElement,
		AActor* connectionElementActor
		);

	void LerpActorTransform(
		AActor* targetActor,
		discenfw::LocalTransform transform1,
		discenfw::LocalTransform transform2,
		float trim
		);

	AActor* CreateNewEntity(
		const std::string& entityClass,
		const std::string& entityType,
		const std::string& entityName,
		AActor* parentActor
		);

	void UpdateElementData(AActor* elemActor, std::shared_ptr< discenfw::Element > elem);

	void UpdateElementTransform(AActor* elemActor, std::shared_ptr< discenfw::Element > elem);

	discenfw::Vector3D ExportVector(const FVector& vec);

	discenfw::CoordSys3D ExportCoordSys(const FVector& uePos, const FRotator& ueRot);

	///@}

private:

	TArray< UConnectionElementComponent* > PendingConnectionElements;


	void BuildConnectionElement(UConnectionElementComponent* PendingConnectionComponent);
	void BuildConnectionElements();
};
