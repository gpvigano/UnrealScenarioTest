// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"

#include "MovableActor.h"
#include "UnrealScenario.h"


#include "EditableConnectionActor.generated.h"

UCLASS(Abstract)
/**
 * Base class for editable connection elements.
 */
class UNREALSCENARIO_API AEditableConnectionActor : public AMovableActor
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Sequence of anchors that define the path.
		*/
		TArray<FConnectionAnchor> ConnectionPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		int32 ActiveAnchorIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Template actor used to create anchor gizmos for path editing.
		*/
		AActor* AnchorHandleTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/** Template actor used to create sections (if SectionBlueprint is not defined). */
		AActor* SectionTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/** Template blueprint used to create sections. */
		TSubclassOf<AActor> SectionBlueprint;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Use internal mouse interaction.
		*/
		bool bMouseInteractionEnabled = true;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Let the pipeline connect to elements sockets with mouse interaction.
		*/
		bool bSocketConnectionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario", meta = (UIMin = 0.01, UIMax = 1.0))
		/**
		If positive this is a delay in seconds before the connection components are updated, else no delay is applied.
		*/
		float UpdateDelay;

	UPROPERTY(VisibleAnywhere, AdvancedDisplay, BlueprintReadOnly, Category = "Unreal Scenario")
		/**
		Sections of the connection element.
		*/
		TArray<AActor*> ConnectionSections;

	UPROPERTY(VisibleAnywhere, AdvancedDisplay, BlueprintReadOnly, Category = "Unreal Scenario")
		TArray<AActor*> AnchorHandles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/** Try to align anchors along the closer axis while mouse dragging. */
		bool bSnapToAxis = false;



	AEditableConnectionActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual bool ShouldTickIfViewportsOnly() const override;

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/** Check if the given index is valid for the current connection path. */
		virtual bool IsValidAnchorIndex(int32 PointIndex);

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/** Select the anchor with the given index or deselect all if the index is negative. */
		virtual void SelectAnchor(int32 PointIndex);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/** Return the actor of the active hadle (it can be null). */
		AActor* GetActiveAnchorHandle()
	{
		return ActiveAnchorHandle;
	}

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/** Delete the all the sections and rebuild them according to the connection path. */
		void RebuildConnectionSections();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/** Add a new section (useful when there are no sections). */
		virtual void AddSection();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/** Insert a new section before the active anchor. */
		virtual int32 InsertSectionBefore();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/** Insert a new anchor and section after the active anchor. */
		virtual int32 InsertSectionAfter();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/** Delete the current anchor and rebuild the sections. */
		virtual void DeleteAnchor();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/** Delete the all the sections and reset the connection path. */
		virtual void ClearAll();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/** Update the sections according to the connection path. */
		virtual void UpdateSections();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/** Update the sections only if something was changed. */
		virtual void UpdateOnlyIfNeeded();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/** Show the given handle if valid, hide the other handles. */
		virtual void HighlightHandle(AActor* PointHandle);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/** Check if an handle is highlighted for editing. */
		virtual bool IsEditing();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/** Show/hide the active handle (if any), hide the other handles. */
		void ToggleActiveHandle(bool bShown);

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/** Enable/disable the anchor handles. */
		void SetHandlesEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/** Select the given anchor handle (if valid), hide the other handles. */
		virtual bool SelectHandle(AActor* PointHandle);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/** Select the previous anchor (loop to the last if at the beginning). */
		virtual void PreviousAnchor();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/** Select the next anchor (loop to the first if at the end). */
		virtual void NextAnchor();

	UFUNCTION()
		/** Event handler for mouse over a control point handle. */
		void OnBeginMouseOverHandle(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
		/** Event handler for mouse leaving a control point handle. */
		void OnEndMouseOverHandle(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
		/** Event handler for mouse click on a control point handle. */
		void OnMouseClickedHandle(UPrimitiveComponent* ClickedComp, FKey ButtonPressed);

protected:

	// Timer used to delay the update of the connector shape while it is moving.
	FTimerHandle TimerHandle;
	FVector PrevHandlePos;
	FVector PrevPos;

	// Flag set by the timer to signal the need for an update.
	bool bUpdateNeeded = false;

	// Tells if at least one anchor is linked to the world.
	bool bAnchoredToWorld = false;

	// Tells if at least one anchor is linked to another element.
	bool bAnchoredToElements = false;

	// Tells if at least one anchor handle is overlapping another anchor.
	bool bHandleOverlapped = false;

	// Tells if there is an active axis snapping modifier.
	bool bChangeSnapToAxis = false;

	// Reference to the active anchor handle.
	AActor* ActiveAnchorHandle;

	// Reference to the current highlighted anchor handle.
	AActor* HighlightedHandle = nullptr;

	// Reference to the current moving anchor handle.
	AActor* MovingHandle = nullptr;

	// Copy of the connection path used for cancelling.
	TArray<FConnectionAnchor> SavedConnectionPath;

	// Distance of the selected anchor handle when it was picked with the mouse cursor.
	float PickedHandleDist;

	// Primitive component clicked.
	UPrimitiveComponent* ClickedComponent = nullptr;

	// Socket anchor that will be connected if the current operation is confirmed.
	FConnectionAnchor CandidateSocketAnchor;

	// Used to detect changes in the positions of the anchors.
	TMap<USceneComponent*, FVector> LastAnchorPositions;



	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called whenever this actor is being removed from a level
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
		void OnHandleOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnHandleOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**
	Check if the section template, used to build the connector geometry, is defined.
	*/
	virtual bool CheckSectionTemplate();

	/**
	Destroy the connection sections, calling DestroySectionsImpl().
	*/
	virtual void DestroyConnectionSections();

	/**
	Build the connection sections, calling BuildSectionsImpl().
	*/
	virtual void BuildConnectionSections();

	/**
	Build the connection sections, old sections are destroyed, calling CreateSectionActor() foreach section, then calling UpdateSections().
	*/
	virtual void BuildSectionsImpl();

	/**
	Update sections.
	@note this method must be overridden by each derieved class.
	*/
	virtual void UpdateSectionsImpl()
	{
	}

	/**
	Destroy the connection sections, calling UUEUtil::DestroyActorsArray().
	*/
	virtual void DestroySectionsImpl();

	/**
	Insert a connection section with the given achor point location, at the given index in the sections list.
	*/
	virtual void InsertSection(FVector location, int32 index);

	/**
	Delete the connection section at the given index in the sections list.
	*/
	virtual void DeleteSection(int32 pointIndex);

	/**
	Create a new connection section based on the section template.
	*/
	AActor* CreateSectionActor();

	/**
	Create a new control point handle based on the handle template.
	*/
	AActor* CreateAnchorHandleActor();

	/**
	Prepare and set up a control point handle for interaction.
	*/
	void SetupAnchorHandleActor(AActor* actor);

	/**
	Update anchor handles locations according to anchor control points.
	*/
	virtual void UpdateAnchorHandles();

	/**
	Compute the location of an anchor when it is dragged, according to snap modifiers.
	*/
	virtual FVector ComputeAnchorLocation(FVector HandleLocalPosition, bool bSnapToAxisFlag);

	// (Re)start the update timer.
	void StartUpdateTimer();

	// Simply set a flag to enable the update in Tick()
	void OnUpdateTimer();

	// Get the position of the active handle in local coordinates.
	FVector GetHandleLocalPosition();

	// Get the position of the active handle in world coordinates.
	FVector GetHandleLocation();

	// Get the position of the active connection anchor in local coordinates.
	FVector GetAnchorLocalPosition(int AnchorIndex);

	// Set the position of the active connection anchor in local coordinates.
	void SetAnchorLocalPosition(int AnchorIndex, const FVector& Position);

	// Initialize dragging for the given anchor handle.
	bool DragHandle(AActor* handle);

	// Update the dragged anchor handle according to the corresponding connection anchor.
	void UpdateMovingHandle();

	// Update the mouse interaction (drag, rotate, push/pull, etc.)
	void UpdateMouseInteraction();

	// Update the interaction with a VR device (not yet implemented)
	void UpdateVrInteraction();

	// Called when the user confirms an interactive operation.
	virtual void OnEditConfirm();

	// Called when an anchor is added before the current one.
	virtual void OnEditAddBefore();

	// Called when an anchor is after before the current one.
	virtual void OnEditAddAfter();

	// Called when the user cancels an interactive operation.
	virtual void OnEditCancel();

	// Log information about the active anchor (if any).
	void PrintAnchorInfo();

	// If this actor has an entity data component, update its Modified property.
	void SetModified(bool bModified);

	// Highlight the anchor that will be linked if the current operation is confirmed.
	void HighlightCandidateAnchor(bool bHighlighted);
};
