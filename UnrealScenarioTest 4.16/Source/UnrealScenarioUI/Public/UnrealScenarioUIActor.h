// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "UnrealScenarioXpActor.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

class UUScenUIMain;
class UUScenUIConfig;


#include "UnrealScenarioUIActor.generated.h"

UCLASS(ClassGroup = UnrealScenarioUi)
/**
 * Actor used to control a user interface for Unreal Scenario module.
 */
class UNREALSCENARIOUI_API AUnrealScenarioUIActor : public AUnrealScenarioXpActor
{
	GENERATED_BODY()

public:

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Unreal Scenario|User Interface")
		/**
		Actor used to display labels
		*/
		AActor* TooltipLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario|User Interface")
		/**
		Material used to highlight forbidden actions with icons
		*/
		UMaterialInterface* IconMaterialForbidden;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario|User Interface")
		/**
		Material used to highlight suggested actions with icons
		*/
		UMaterialInterface* IconMaterialSuggested;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario|User Interface")
		/**
		Widget blueprint for the main control interface
		*/
		TSubclassOf<UUserWidget> MainUiBlueprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario|User Interface")
		/**
		Widget blueprint for the configuration interface
		*/
		TSubclassOf<UUserWidget> ConfigurationUiBlueprint;



	AUnrealScenarioUIActor();

	virtual void Tick(float DeltaTime) override;

	/**
	Start a new episode (current episode, if not completed, is discarded)
	*/
	virtual void NewEpisode() override;

	/**
	Update visual hints about suggested/forbidden actions
	*/
	virtual void UpdateHints() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
		Open configuration user interface
		*/
		virtual void ToggleConfigurationUI(bool bEnabled);

protected:

	UPROPERTY()
		// Widget BP for the main UI panel
		UUScenUIMain* MainUiWidget = nullptr;

	UPROPERTY()
		// Widget BP for the configuration UI panel
		UUScenUIConfig* ConfigUiWidget = nullptr;


	virtual void BeginPlay() override;

	virtual void OnScenarioLoaded() override;

	virtual void OnEntityAdded() override;

	virtual void OnEntityRemoved() override;

	virtual void OnActionDone(bool bCompletedEpisode, bool bUpdateScene = true) override;


	virtual void UpdateMainUI();

	virtual void UpdateOutcomeText();

};
