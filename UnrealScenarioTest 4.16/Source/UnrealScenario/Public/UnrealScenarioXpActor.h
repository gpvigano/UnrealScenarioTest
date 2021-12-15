// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "UnrealScenarioActor.h"

#include "EntityStateComponent.h"

#include "UnrealScenarioXpActor.generated.h"

UCLASS(ClassGroup = UnrealScenario)
/**
 * Actor implementing the experience management for Unreal Scenario module.
 */
class UNREALSCENARIO_API AUnrealScenarioXpActor : public AUnrealScenarioActor
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario|Xp")
		/**
		Path of the experience file (JSON) relative to project or build root.
		*/
		FString XpFileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario|Xp")
		/**
		Path of the learning configuration file (JSON) relative to project or build root.
		*/
		FString RLConfigFileName = "default_RLConfig.json";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario|Xp")
		/**
		Name of the current goal.
		*/
		FString CurrentGoalName;


	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Unreal Scenario|Xp")
		/**
		Enable automatic training.
		*/
		bool bAutoTrainingEnabled = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Unreal Scenario|Xp")
		/**
		Stop automatic training on the next successful episode.
		*/
		bool bStopAutoTrainingOnCompletion = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Unreal Scenario|Xp")
		/**
		Enable live scene update during automatic training.
		*/
		bool bAutoTrainingLiveUpdate = true;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Unreal Scenario|Xp", meta = (ClampMin = "0"), meta = (ClampMax = "2"))
		/**
		Automatic training speed (seconds between two steps).
		*/
		float AutoTrainingStepDelay = 0.5f;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Unreal Scenario|Xp")
		/**
		Enable loop detection for automatic training.
		*/
		bool bAutoTrainingLoopDetection = true;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Unreal Scenario|Xp")
		/**
		Enable learning for automatic training.
		*/
		bool bLearningEnabled = true;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Unreal Scenario|Xp")
		/**
		Enable experience update during automatic training.
		*/
		bool bAutoTrainingUpdateXp = true;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Unreal Scenario|Xp")
		/**
		Stop automatic training at first success.
		*/
		bool bAutoTrainingStopOnSuccess = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Unreal Scenario|Xp")
		/**
		Optimize the experience before saving
		(disable this to get the full experience saved).
		*/
		bool bOptimizeSavedXp = true;

	UPROPERTY(EditInstanceOnly, Category = "Unreal Scenario|Xp")
		/**
		Agent statistics updated during automatic training
		*/
		FAgentStats Statistics;




	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
	/**
	Synchronize the scene actors with the scenario state. This method calls SyncEntityState() and SyncRelationships() for each entity.
	*/
		virtual void SyncSceneState();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
	/**
	Update visual hints about suggested/forbidden actions. This method must be implemented in a derived class.
	*/
	virtual void UpdateHints() { NotifyMissingImplementation(); }

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the current goal
		*/
		FString GetCurrentGoal();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the list of goals
		*/
		TArray<FString> GetGoals();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Set the current goal, create it if needed
		*/
		void SetCurrentGoal(FString GoalName);

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Set the current goal, create it if needed
		*/
		bool RenameCurrentGoal(FString GoalName);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the success conditions for current goal
		*/
		void GetSuccessConditions(TArray<FPropertyCondition>& SuccessPropConditions, TArray<FRelationshipCondition>& SuccessRelConditions);

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Set the success conditions for the current goal
		*/
		void SetSuccessConditions(TArray<FPropertyCondition> SuccessConditions, TArray<FRelationshipCondition> SuccessRelConditions);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the failure conditions for current goal
		*/
		void GetFailureConditions(TArray<FPropertyCondition>& FailurePropConditions, TArray<FRelationshipCondition>& FailureRelConditions);

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Set the failure conditions for the current goal
		*/
		void SetFailureConditions(TArray<FPropertyCondition> FailureConditions, TArray<FRelationshipCondition> FailureRelConditions);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the scenario rewards for current goal
		*/
		FScenarioRewards GetScenarioRewards();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Set the scenario rewards for current goal
		*/
		void SetScenarioRewards(FScenarioRewards ScenarioRewards);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the type of the given entity
		*/
		FString GetEntityType(FString EntityId);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the default properties of the given entity type
		*/
		TArray<FString> GetEntityTypeDefaultProperties(FString EntityTypeId);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the predefined values of the given default properties of the given entity type
		*/
		TArray<FString> GetEntityTypeDefaultPropertyValues(FString EntityTypeId, FString PropertyName);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the possible properties for the given entity type, including special tags
		*/
		TArray<FString> GetPropertyChoices(FString EntityId, FString EntityTypeName);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the possible values of the given default properties
		of the given entity type, including special tags.
		*/
		TArray<FString> GetPropertyValueChoices(FString EntityId, FString PropertyName, FString EntityTypeName);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the statistics for current agent.
		*/
		FAgentStats GetAgentStatistics();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Reset the statistics for current agent.
		*/
		void ResetAgentStatistics();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Reset the current agent, statistics included.
		*/
		void ResetAgent();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the reward for the last stored scenario state for the current episode (0 if no episode is defined).
		*/
		int GetLastScenarioStateReward();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the learning configuration for current agent.
		*/
		FRLConfig GetRLConfiguration();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Set the learning configuration for current agent.
		*/
		void SetRLConfiguration(FRLConfig RLConfiguration);

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Load the learning configuration for current agent.
		*/
		bool LoadRLConfiguration();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Save the learning configuration for current agent.
		*/
		bool SaveRLConfiguration();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Set ignored/considered the system failure conditions for the given assistant.
		*/
		bool SetSystemFailureIgnored(bool bIgnored);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Check if the system failure conditions for the given assistant are ignored.
		*/
		bool IsSystemFailureIgnored();


	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Set the discounting constant for the current experience.
		*/
		bool SetDiscountingConstant(float DiscountingConstant);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the discounting constant for the current experience.
		*/
		float GetDiscountingConstant() const;

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		void SetAutoTrainingLiveUpdate(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		void SetAutoTraining(bool bEnabled, bool bStopOnCompletion, bool bStopOnSuccess);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
		Start a new episode (current episode, if not completed, is discarded).
		*/
		virtual void NewEpisode();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
		Perform an automatic training step.
		*/
		virtual void TrainingStep();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
		Load the experience.
		*/
		virtual void LoadExperience();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
		Save the current experience.
		*/
		virtual void SaveExperience();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
		Clear the current experience.
		*/
		virtual void ClearExperience();

protected:

	// Time elapsed since the last training step
	float AutoTrainingChrono = 0.0f;

	std::string AgentName = "Default";
	std::string CyberSystemName;

	// Initialization of the experience manager
	bool bXpInitialized = false;



	virtual void BeginPlay() override;

	virtual void InitDigitalScenarioFramework() override;


	virtual void OnScenarioLoaded() override;

	virtual void OnEntityAdded() override;

	virtual void OnEntityRemoved() override;

	/**
	Initialize digital system. CyberSystemName must be defined in a derived class.
	*/
	virtual void InitCyberSystem();

	/**
	Initialize experience. This method must be implemented in a derived class.
	*/
	virtual void InitXp();

	/**
	Execute and process an action, call OnActionDone().
	*/
	virtual void TakeAction(discenfw::xp::Action ChosenAction);

	/**
	Called after an action is executed and processed.
	*/
	virtual void OnActionDone(bool bCompletedEpisode, bool bUpdateScene = true);

	/**
	Synchronize a state component with an entity state. This method must be implemented in a derived class.
	*/
	virtual void SyncEntityState(
		const std::shared_ptr<discenfw::Entity> ScenEntity,
		const std::shared_ptr<discenfw::xp::EntityState> XpEntityState,
		UEntityStateComponent* CompState)
	{
		NotifyMissingImplementation();
	}

	/**
	Synchronize the scene with the relationships of an entity state. This method must be implemented in a derived class.
	*/
	virtual void SyncRelationships(
		const std::shared_ptr<discenfw::xp::EnvironmentState> XpScenarioState)
	{
		NotifyMissingImplementation();
	}

	void NotifyMissingImplementation();

};
