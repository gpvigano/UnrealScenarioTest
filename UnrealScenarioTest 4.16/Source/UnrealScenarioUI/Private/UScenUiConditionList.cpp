// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "UScenUiConditionList.h"


#include "UnrealScenarioUIActor.h"
#include "UEUtil.h"
#include "UScenUtil.h"


FString UUScenUiConditionList::EntitySelectorToString(
	const FString& EntityId,
	const FString& TypeName
	)
{
	FString encodedString;
	if (!UUScenUtil::IsSpecialEntityTag(EntityId))
	{
		return EntityId;
	}
	if (EntityId == UUScenUtil::GetAllEntitiesTag())
	{
		if (TypeName.IsEmpty())
		{
			encodedString = TEXT("(All entities)");
		}
		else
		{
			// TODO: English plurals ending with 'es'
			encodedString = TEXT("(All ") + TypeName + TEXT("s)");
		}
	}
	else //if (PropertyCondition.EntityId == UUScenUtil::GetAnyEntitiesTag())
	{
		if (TypeName.IsEmpty())
		{
			encodedString = TEXT("(Any entity)");
		}
		else
		{
			encodedString = TEXT("(Any ") + TypeName + TEXT(")");
		}
	}

	return encodedString;
}


bool UUScenUiConditionList::StringToEntitySelector(
	const FString& SelectorString,
	FString& OutEntityName,
	FString& OutEntityTypeName
	)
{
	FString allTag = UUScenUtil::GetAllEntitiesTag();
	FString anyTag = UUScenUtil::GetAnyEntityTag();
	if (SelectorString == TEXT("(All entities)"))
	{
		OutEntityName = allTag;
		OutEntityTypeName = FString();
		return true;
	}
	if (SelectorString == TEXT("(Any entity)"))
	{
		OutEntityName = anyTag;
		OutEntityTypeName = FString();
		return true;
	}
	bool isAny = SelectorString.StartsWith(TEXT("(Any "));
	bool isAll = !isAny && SelectorString.StartsWith(TEXT("(All "));
	if (!isAny&&!isAll)
	{
		OutEntityName = SelectorString;
		OutEntityTypeName.Reset();
		return false;
	}
	OutEntityName = isAll ? allTag : anyTag;
	OutEntityTypeName = SelectorString.Mid(5, SelectorString.Len() - 5 - (isAll ? 2 : 1));

	return true;
}



TArray<FString> UUScenUiConditionList::GetEntitySelectorStrings(
	bool IncludeAll,
	bool IncludeAny,
	bool IncludeScenarioEntities
	)
{
	TArray<FString> encodedEntitySelectors;
	if (!UnrealScenarioUIActor)
	{
		Init();
	}

	if (UnrealScenarioUIActor)
	{
		if (IncludeScenarioEntities)
		{
			encodedEntitySelectors = UnrealScenarioUIActor->GetEntityIdentifiers();
		}

		if (IncludeAll || IncludeAny)
		{
			TArray<FString> stateTypeNames = UUScenUtil::GetEntityStateTypeNames();
			FString allTag = UUScenUtil::GetAllEntitiesTag();
			FString anyTag = UUScenUtil::GetAnyEntityTag();
			for (FString stateTypeName : stateTypeNames)
			{
				FPropertyCondition selector;
				selector.TypeName = stateTypeName;
				if (IncludeAll)
				{
					selector.EntityId = allTag;
					encodedEntitySelectors.Add(EntitySelectorToString(selector.EntityId,selector.TypeName));
				}
				if (IncludeAny)
				{
					selector.EntityId = anyTag;
					encodedEntitySelectors.Add(EntitySelectorToString(selector.EntityId,selector.TypeName));
				}
			}
			if (IncludeAll)
			{
				encodedEntitySelectors.Add(TEXT("(All entities)"));
			}
			if (IncludeAny)
			{
				encodedEntitySelectors.Add(TEXT("(Any entity)"));
			}
		}
	}
	return encodedEntitySelectors;
}



FString UUScenUiConditionList::ComparisonOperatorToString(
	EPropertyComparisonOperator ComparisonOperator
	)
{
	using namespace discenfw;
	FString encodedOperator = CompOpToString((CompOp)ComparisonOperator);
	return encodedOperator;
}


TArray<FString> UUScenUiConditionList::GetComparisonOperatorStrings()
{
	using namespace discenfw;
	TArray<FString> encodedEntitySelectors;
	for (int i = (int)EPropertyComparisonOperator::Equal; i < (int)EPropertyComparisonOperator::Defined; i++)
	{
		encodedEntitySelectors.Add(CompOpToString((CompOp)i));
	}
	return encodedEntitySelectors;
}


bool UUScenUiConditionList::SplitEntitySelector(
	const FString& EncodedString,
	FString& OutEntityName,
	FString& OutEntityTypeName
	)
{
	FString allTag = UUScenUtil::GetAllEntitiesTag();
	FString anyTag = UUScenUtil::GetAnyEntityTag();
	if (EncodedString == TEXT("(All entities)"))
	{
		OutEntityName = allTag;
		OutEntityTypeName = FString();
		return true;
	}
	if (EncodedString == TEXT("(Any entity)"))
	{
		OutEntityName = anyTag;
		OutEntityTypeName = FString();
		return true;
	}
	bool isAny = EncodedString.StartsWith(TEXT("(Any "));
	bool isAll = !isAny && EncodedString.StartsWith(TEXT("(All "));
	if (!isAny&&!isAll)
	{
		OutEntityName = EncodedString;
		OutEntityTypeName.Reset();
		return false;
	}
	OutEntityName = isAll ? allTag : anyTag;
	OutEntityTypeName = EncodedString.Mid(5, EncodedString.Len() - 5 - (isAll ? 2 : 1));

	return true;
}



TArray<FString> UUScenUiConditionList::GetPropertiesFromSelector(
	const FString& EncodedString
	)
{
	TArray<FString> propertyNames;
	if (EncodedString.IsEmpty())
	{
		return propertyNames;
	}
	if (!UnrealScenarioUIActor)
	{
		Init();
	}

	if (UnrealScenarioUIActor)
	{
		FString entityName;
		FString entityTypeName;
		SplitEntitySelector(EncodedString, entityName, entityTypeName);
		propertyNames = UnrealScenarioUIActor->GetPropertyChoices(entityName, entityTypeName);
	}
	return propertyNames;
}


TArray<FString> UUScenUiConditionList::GetPropertyValuesFromSelector(
	const FString& EntitySelector,
	const FString& PropertyName
	)
{
	TArray<FString> propertyNames;
	if (!UnrealScenarioUIActor)
	{
		Init();
	}

	if (UnrealScenarioUIActor)
	{
		FString entityName;
		FString entityTypeName;
		SplitEntitySelector(EntitySelector, entityName, entityTypeName);
		propertyNames = UnrealScenarioUIActor->GetPropertyValueChoices(entityName, PropertyName, entityTypeName);
	}
	return propertyNames;
}


void UUScenUiConditionList::SplitPropCondition(
	FPropertyCondition PropertyCondition,
	FString& OutEntitySelector,
	FString& OutPropertyName,
	FString& OutOperatorName,
	FString& OutValue
	)
{
	OutEntitySelector = EntitySelectorToString(PropertyCondition.EntityId,PropertyCondition.TypeName);
	OutPropertyName = PropertyCondition.PropertyId;
	OutOperatorName = ComparisonOperatorToString(PropertyCondition.ComparisonOperator);
	OutValue = PropertyCondition.PropertyValue;
}


FPropertyCondition UUScenUiConditionList::BuildPropCondition(
	const FString& SelectedEntity,
	const FString& SelectedProperty,
	const FString& SelectedOperator,
	const FString& SelectedValue
	)
{
	using namespace discenfw;
	FPropertyCondition condition;
	SplitEntitySelector(SelectedEntity, condition.EntityId, condition.TypeName);
	condition.PropertyId = SelectedProperty;
	condition.ComparisonOperator = (EPropertyComparisonOperator)CompOpFromString(UUEUtil::ToStdString(SelectedOperator));
	condition.PropertyValue = SelectedValue;
	return condition;
}


void UUScenUiConditionList::SplitRelCondition(
	FRelationshipCondition RelationshipCondition,
	FString& OutEntitySelector,
	FString& OutLink,
	FString& OutRelationType,
	FString& OutRelatedEntity,
	FString& OutRelatedLink
	)
{
	OutEntitySelector = EntitySelectorToString(
		RelationshipCondition.EntityId,RelationshipCondition.TypeName);
	OutLink = RelationshipCondition.LinkId;
	OutRelationType = RelationshipCondition.Unrelated ? TEXT(" | ") : TEXT("-->");
	OutRelatedEntity = EntitySelectorToString(RelationshipCondition.RelatedEntityId,FString());
	OutRelatedLink = RelationshipCondition.RelatedLinkId;
}


FRelationshipCondition UUScenUiConditionList::BuildRelCondition(
	const FString& SelectedEntity,
	const FString& SelectedLink,
	const FString& SelectedRelationType,
	const FString& SelectedRelatedEntity,
	const FString& SelectedRelatedLink
	)
{
	using namespace discenfw;
	FRelationshipCondition condition;
	StringToEntitySelector(SelectedEntity, condition.EntityId, condition.TypeName);
	condition.LinkId = SelectedLink;
	condition.Unrelated = (SelectedRelationType == TEXT(" | "));
	condition.RelatedEntityId = SelectedRelatedEntity;
	condition.RelatedLinkId = SelectedRelatedLink;
	return condition;
}

