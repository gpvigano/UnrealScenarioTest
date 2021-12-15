// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "UScenUIBase.h"
#include "UScenUiConditionList.generated.h"

UCLASS(ClassGroup = UnrealScenarioUi)
/**
 * Base class for condition list box blueprints.
 */
class UNREALSCENARIOUI_API UUScenUiConditionList : public UUScenUIBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		FString EntitySelectorToString(const FString& EntityId, const FString& TypeName);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		bool StringToEntitySelector(
			const FString& SelectorString,
			FString& OutEntityName,
			FString& OutEntityTypeName
			);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		TArray<FString> GetEntitySelectorStrings(
			bool IncludeAll = true,
			bool IncludeAny = true,
			bool IncludeScenarioEntities = true
			);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		FString ComparisonOperatorToString(EPropertyComparisonOperator ComparisonOperator);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		TArray<FString> GetComparisonOperatorStrings();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		bool SplitEntitySelector(
			const FString& EncodedString,
			FString& OutEntityName,
			FString& OutEntityTypeName
			);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		TArray<FString> GetPropertiesFromSelector(const FString& EncodedString);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		TArray<FString> GetPropertyValuesFromSelector(
			const FString& EntitySelector,
			const FString& PropertyName
			);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		void SplitPropCondition(
			FPropertyCondition PropertyCondition,
			FString& OutEntitySelector,
			FString& OutPropertyName,
			FString& OutOperatorName,
			FString& OutValue
			);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		FPropertyCondition BuildPropCondition(
			const FString& SelectedEntity,
			const FString& SelectedProperty,
			const FString& SelectedOperator,
			const FString& SelectedValue
			);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		void SplitRelCondition(
			FRelationshipCondition RelationshipCondition,
			FString& OutEntitySelector,
			FString& OutLink,
			FString& OutRelationType,
			FString& OutRelatedEntity,
			FString& OutRelatedLink
			);

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		FRelationshipCondition BuildRelCondition(
			const FString& SelectedEntity,
			const FString& SelectedLink,
			const FString& SelectedRelationType,
			const FString& SelectedRelatedEntity,
			const FString& SelectedRelatedLink
			);
};
