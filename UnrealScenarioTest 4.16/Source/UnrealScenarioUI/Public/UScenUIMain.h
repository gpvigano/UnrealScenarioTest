// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "UScenUIBase.h"
#include "UnrealScenario/Public/CatalogItemWidget.h"



#include "UScenUIMain.generated.h"

class UPanelWidget;
class UComboBoxString;

/**
 * User interface for Unreal Scenario test - Main UI panel.
 */
UCLASS(ClassGroup = UnrealScenarioUi)
class UNREALSCENARIOUI_API UUScenUIMain : public UUScenUIBase
{
	GENERATED_BODY()
	
	
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario|User Interface")
	/**
	Widget blueprint for the main control interface.
	*/
		TSubclassOf<UCatalogItemWidget> CatalogItemWidgetBlueprint;

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Fill the catalog with all the items in catalog data.
		*/
		void FillCatalog(UPanelWidget* Panel);

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Fill the catalog with all the items in catalog data and the filter combo box.
		*/
		void FillCatalogAndFilter(UPanelWidget* Panel, UComboBoxString* FilterComboBox);

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		/**
		Fill the catalog with the items in catalog data with the category equal to the given filter.
		*/
		void FilterCatalog(UPanelWidget* Panel, FString Filter);


	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		void UpdateOutcomeUI(FString OutcomeString, FString RewardString, FString PerformanceString);

	UFUNCTION(BlueprintImplementableEvent, Category = "Unreal Scenario")
		/**
		Event to be implemeted by a derived BP to display the outcome text.
		*/
		void UpdateOutcomeText();

	UFUNCTION(BlueprintImplementableEvent, Category = "Unreal Scenario")
		/**
		Event to be implemeted by a derived BP to display the outcome text.
		*/
		void UpdateStats();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the outcome converted to a Text.
		*/
		FText GetOutcomeText();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the last reward converted to a Text.
		*/
		FText GetRewardText();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		/**
		Get the last performance converted to a Text.
		*/
		FText GetPerformanceText();

	UFUNCTION(BlueprintImplementableEvent, Category = "Unreal Scenario")
		/**
		Event to be implemeted by a derived BP to check/uncheck an 'Auto Training Enabled' check box.
		*/
		void UpdateAutoTrainingEnabledCheckBox();

protected:

	UPROPERTY(Transient)
		FString OutcomeText;

	UPROPERTY(Transient)
		FString RewardText;

	UPROPERTY(Transient)
		FString PerformanceText;

	UCatalogItemWidget* CreateItemWidget();
	
};
