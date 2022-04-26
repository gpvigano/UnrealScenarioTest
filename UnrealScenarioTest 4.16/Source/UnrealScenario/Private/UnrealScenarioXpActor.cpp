// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "UnrealScenarioXpActor.h"
#include "UEUtil.h"
#include "UScenUtil.h"
#include "EntityStateComponent.h"

#include "Misc/Paths.h" // FPaths
#include "Runtime/Launch/Resources/Version.h" // ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION

#include <discenfw/ve/VirtualEnvironmentAPI.h>
#include <discenfw/ve/VeManager.h>
#include <discenfw/DigitalScenarioFramework.h>
#include <discenfw/xp/EnvironmentModel.h>


namespace
{
	using namespace discenfw;
	using namespace discenfw::xp;

	// TODO: functions here are used to avoid declarations in the header file, consider a translation of these functions to methods, once all the involved classes are protected with a cross boundary safe interface

	void ConvertPropertyCondition(const PropertyCondition& cond, FPropertyCondition& propertyCondition)
	{
		propertyCondition.PropertyId = UUEUtil::ToFString(cond.PropertyName);
		propertyCondition.PropertyValue = UUEUtil::ToFString(cond.PropertyValue);
		propertyCondition.ComparisonOperator = (EPropertyComparisonOperator)(cond.ComparisonOperator);
	}

	void ConvertRelationshipCondition(const RelationshipCondition& cond, FRelationshipCondition& propertyCondition)
	{
		propertyCondition.LinkId = UUEUtil::ToFString(cond.RelatingLinkId);
		propertyCondition.RelatedEntityId = UUEUtil::ToFString(cond.Related.EntityId);
		propertyCondition.RelatedLinkId = UUEUtil::ToFString(cond.Related.LinkId);
		propertyCondition.Unrelated = cond.Unrelated;
	}


	void ConvertEntityCondition(
		const EntityCondition& entCond,
		TArray<FPropertyCondition>& conditions,
		TArray<FRelationshipCondition>& relConditions
		)
	{
		if (!entCond.PropConditions.empty())
		{
			FPropertyCondition entPropCond;
			entPropCond.EntityId = UUEUtil::ToFString(entCond.EntityId);
			entPropCond.TypeName = UUEUtil::ToFString(entCond.TypeName);

			//TODO: support for multiple property conditions

			ConvertPropertyCondition(entCond.PropConditions[0], entPropCond);
			conditions.Add(entPropCond);
		}
		if (!entCond.RelConditions.empty())
		{
			FRelationshipCondition entRelCond;
			entRelCond.EntityId = UUEUtil::ToFString(entCond.EntityId);
			entRelCond.TypeName = UUEUtil::ToFString(entCond.TypeName);

			//TODO: support for multiple relationship conditions

			ConvertRelationshipCondition(entCond.RelConditions[0], entRelCond);
			relConditions.Add(entRelCond);
		}
	}


	void ConvertCondition(
		const Condition& cond,
		TArray<FPropertyCondition>& conditions,
		TArray<FRelationshipCondition>& relConditions
		)
	{
		for (const auto& entCond : cond.GetEntityConditions())
		{
			ConvertEntityCondition(entCond,conditions,relConditions);
		}
	};


	bool ConditionToFConditions(
		const Condition& cond,
		TArray<FPropertyCondition>& conditions,
		TArray<FRelationshipCondition>& relCond,
		LogicOp compOp)
	{ 
		ConvertCondition(cond, conditions, relCond);

		// handle related conditions

		for (const auto& condItr : cond.GetRelatedConditions())
		{
			if (condItr.first != compOp)
			{
				FString compOpStr = UUEUtil::ToFString(LogicOpToString(compOp));
				UUScenUtil::Log(Logger::ERROR, TEXT("Only ") + compOpStr + TEXT(" operator is supported for related conditions."));
				return false;
			}
			ConvertCondition(*(condItr.second), conditions, relCond);
		}
		return true;
	}


	bool ConditionToFPropertyCondition(
		const Condition& cond, TArray<FPropertyCondition>& conditions, LogicOp compOp)
	{ 
		TArray<FRelationshipCondition> relCond;
		return ConditionToFConditions(cond, conditions, relCond, compOp);
	}


	PropertyCondition ToPropertyCondition(const FPropertyCondition& propCond)
	{
		std::string prop = UUEUtil::ToStdString(propCond.PropertyId);
		CompOp op = (propCond.ComparisonOperator == EPropertyComparisonOperator::Equal) ? CompOp::EQUAL : CompOp::DIFFERENT;
		std::string val = UUEUtil::ToStdString(propCond.PropertyValue);
		return PropertyCondition({ prop, op, val });
	}


	RelationshipCondition ToRelationshipCondition(const FRelationshipCondition& relCond)
	{
		std::string link = UUEUtil::ToStdString(relCond.LinkId);
		std::string related = UUEUtil::ToStdString(relCond.RelatedEntityId);
		std::string relatedLink = UUEUtil::ToStdString(relCond.RelatedLinkId);

		return RelationshipCondition(link, related, relatedLink, relCond.Unrelated);
	}


	void ToCondition(const FPropertyCondition& propCond, Condition& condition)
	{
		std::string id = UUEUtil::ToStdString(propCond.EntityId);
		CompOp op = (propCond.ComparisonOperator == EPropertyComparisonOperator::Equal) ? CompOp::EQUAL : CompOp::DIFFERENT;
		EntityCondition entityCondition
		(
			id,
			{
				PropertyCondition(
					UUEUtil::ToStdString(propCond.PropertyId),
					op,
					UUEUtil::ToStdString(propCond.PropertyValue)
				)
			},
			{
			},
			UUEUtil::ToStdString(propCond.TypeName)
		);
		condition.SetEntityCondition(entityCondition);
	}


	void ToCondition(const FRelationshipCondition& propCond, Condition& condition)
	{
		std::string id = UUEUtil::ToStdString(propCond.EntityId);
		EntityCondition entityCondition
		(
			id,
			{
			},
			{
				RelationshipCondition(
				UUEUtil::ToStdString(propCond.LinkId),
				UUEUtil::ToStdString(propCond.RelatedEntityId),
				UUEUtil::ToStdString(propCond.RelatedLinkId),
				propCond.Unrelated
				)
			}
		);
		condition.SetEntityCondition(entityCondition);
	}
}


void AUnrealScenarioXpActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bAutoTrainingEnabled)
	{
		// execute training steps at given intervals

		AutoTrainingChrono += DeltaTime;
		if (AutoTrainingChrono > AutoTrainingStepDelay)
		{
			AutoTrainingChrono = 0.0f;
			TrainingStep();
		}
	}
	else
	{
		AutoTrainingChrono = 0.0f;
	}
}


void AUnrealScenarioXpActor::SyncSceneState()
{
	CancelEditing();
	using namespace discenfw;
	using namespace discenfw::xp;

	const auto& scenState = DiScenFw()->GetLastScenarioState(AgentName);
	for (const auto& entStateRef : scenState->GetEntityStates())
	{
		const std::string& entityId = entStateRef.first;
		FString idString = UUEUtil::ToFString(entityId);
		const auto& entState = entStateRef.second;
		AActor* entityActor = GetEntityActorFromId(entityId);
		if (!entityActor)
		{
			UUEUtil::Log.LogMessage(Logger::ERROR, FString::Printf(TEXT("Entity actor %s not found"), *idString), "UnrealScenarioTest");
			continue;
		}
		UEntityStateComponent* compState = Cast<UEntityStateComponent>(entityActor->GetComponentByClass(UEntityStateComponent::StaticClass()));
		if (!compState)
		{
			UUEUtil::Log.LogMessage(Logger::ERROR, FString::Printf(TEXT("Entity State Component not found for %s."), *idString), "UnrealScenarioTest");
			continue;
		}
		using namespace discenfw;
		using namespace discenfw::scen;
		const std::shared_ptr<Entity> ent = DiScenFw()->GetEntity(entityId);
		if (!ent)
		{
			UUEUtil::Log.LogMessage(Logger::ERROR, FString::Printf(TEXT("Entity %s not found"), *idString), "UnrealScenarioTest");
			continue;
		}

		SyncEntityState(ent, entState, compState);
	}

	SyncRelationships(scenState);
}



FString AUnrealScenarioXpActor::GetCurrentGoal()
{
	using namespace discenfw;
	return UUEUtil::ToFString(DiScenFw()->GetCurrentGoal(AgentName));
}


void AUnrealScenarioXpActor::SetCurrentGoal(FString GoalName)
{
	using namespace discenfw;
	if (DiScenFw()->SetCurrentGoal(AgentName, UUEUtil::ToStdString(GoalName)))
	{
		CurrentGoalName = GoalName;
		NewEpisode();
	}
}


bool AUnrealScenarioXpActor::RenameCurrentGoal(FString GoalName)
{
	using namespace discenfw;
	if (DiScenFw()->RenameCurrentGoal(AgentName, UUEUtil::ToStdString(GoalName)))
	{
		CurrentGoalName = GoalName;
		NewEpisode();
		return true;
	}
	return false;
}


TArray<FString> AUnrealScenarioXpActor::GetGoals()
{
	using namespace discenfw;
	const std::vector<std::string>& goalsStd = DiScenFw()->GetGoals(AgentName);
	TArray<FString> goals;
	for (const std::string& goal : goalsStd)
	{
		goals.Add(UUEUtil::ToFString(goal));
	}
	return goals;
}


void AUnrealScenarioXpActor::GetSuccessConditions(TArray<FPropertyCondition>& SuccessPropConditions, TArray<FRelationshipCondition>& SuccessRelConditions)
{
	using namespace discenfw;
	using namespace discenfw::xp;
	const Condition& successCond = DiScenFw()->GetSuccessCondition(AgentName);

	ConditionToFConditions(successCond, SuccessPropConditions, SuccessRelConditions, LogicOp::AND);
}


void AUnrealScenarioXpActor::SetSuccessConditions(
	TArray<FPropertyCondition> SuccessConditions,
	TArray<FRelationshipCondition> SuccessRelConditions
	)
{
	using namespace discenfw;
	using namespace discenfw::xp;
	Condition successCond;

	for (const FPropertyCondition& propCond : SuccessConditions)
	{
		if (successCond.GetEntityConditions().empty())
		{
			ToCondition(propCond, successCond);
		}
		else
		{
			Condition subCond;
			ToCondition(propCond, subCond);
			successCond.AddCondition(LogicOp::AND, subCond);
		}
	}

	for (const FRelationshipCondition& relCond : SuccessRelConditions)
	{
		if (successCond.GetEntityConditions().empty())
		{
			ToCondition(relCond, successCond);
		}
		else
		{
			Condition subCond;
			ToCondition(relCond, subCond);
			successCond.AddCondition(LogicOp::AND, subCond);
		}
	}

	DiScenFw()->SetSuccessCondition(AgentName, successCond);

	// reset statistics
	Statistics = FAgentStats();
	NewEpisode();
}

		
void AUnrealScenarioXpActor::GetFailureConditions(
	TArray<FPropertyCondition>& FailurePropConditions,
	TArray<FRelationshipCondition>& FailureRelConditions)
{
	using namespace discenfw;
	using namespace discenfw::xp;

	const Condition& failureCond = DiScenFw()->GetFailureCondition(AgentName);

	ConditionToFConditions(failureCond, FailurePropConditions, FailureRelConditions, LogicOp::OR);
}


void AUnrealScenarioXpActor::SetFailureConditions(
	TArray<FPropertyCondition> FailureConditions,
	TArray<FRelationshipCondition> FailureRelConditions)
{
	using namespace discenfw;
	using namespace discenfw::xp;

	Condition failureCond;
	for (const FPropertyCondition& propCond : FailureConditions)
	{
		if (failureCond.GetEntityConditions().empty())
		{
			ToCondition(propCond, failureCond);
		}
		else
		{
			Condition subCond;
			ToCondition(propCond, subCond);
			failureCond.AddCondition(LogicOp::OR, subCond);
		}
	}
	for (const FRelationshipCondition& relCond : FailureRelConditions)
	{
		if (failureCond.GetEntityConditions().empty())
		{
			ToCondition(relCond, failureCond);
		}
		else
		{
			Condition subCond;
			ToCondition(relCond, subCond);
			failureCond.AddCondition(LogicOp::OR, subCond);
		}
	}
	DiScenFw()->SetFailureCondition(AgentName, failureCond);

	// reset statistics
	Statistics = FAgentStats();
	NewEpisode();
}


FScenarioRewards AUnrealScenarioXpActor::GetScenarioRewards()
{
	FScenarioRewards scenarioRewards;
	using namespace discenfw;
	using namespace discenfw::xp;
	const auto& dsfRewards = DiScenFw()->GetRewardRules(AgentName);

	// Scenario rewards
	bool hasSuccessReward = dsfRewards.ResultReward.find(ActionResult::SUCCEEDED) != dsfRewards.ResultReward.cend();
	bool hasFailureReward = dsfRewards.ResultReward.find(ActionResult::FAILED) != dsfRewards.ResultReward.cend();
	bool hasDeadlockReward = dsfRewards.ResultReward.find(ActionResult::DEADLOCK) != dsfRewards.ResultReward.cend();
	bool hasInProgressReward = dsfRewards.ResultReward.find(ActionResult::IN_PROGRESS) != dsfRewards.ResultReward.cend();

	if(hasSuccessReward) scenarioRewards.SuccessReward = dsfRewards.ResultReward.at(ActionResult::SUCCEEDED);
	if(hasFailureReward) scenarioRewards.FailureReward = dsfRewards.ResultReward.at(ActionResult::FAILED);
	if(hasDeadlockReward) scenarioRewards.DeadlockReward = dsfRewards.ResultReward.at(ActionResult::DEADLOCK);
	if(hasInProgressReward) scenarioRewards.InProgressReward = dsfRewards.ResultReward.at(ActionResult::IN_PROGRESS);
	
	// EntityConditionRewards
	for (const auto& rewardPair : dsfRewards.EntityConditionRewards)
	{
		if (!rewardPair.first.PropConditions.empty())
		{
			int32 idx = scenarioRewards.EntityPropertyRewards.AddDefaulted(1);
			auto& newItem = scenarioRewards.EntityPropertyRewards[idx];
			newItem.Condition.EntityId = UUEUtil::ToFString(rewardPair.first.EntityId);
			ConvertPropertyCondition(rewardPair.first.PropConditions[0], newItem.Condition);
			newItem.Reward = rewardPair.second;
		}
		if (!rewardPair.first.RelConditions.empty())
		{
			int32 idx = scenarioRewards.EntityRelationshipRewards.AddDefaulted(1);
			auto& newItem = scenarioRewards.EntityRelationshipRewards[idx];
			newItem.Condition.EntityId = UUEUtil::ToFString(rewardPair.first.EntityId);
			ConvertRelationshipCondition(rewardPair.first.RelConditions[0], newItem.Condition);
			newItem.Reward = rewardPair.second;
		}
	}

	// CumulativeRewards
	int32 newItemIndex = scenarioRewards.PropertyCountRewards.AddDefaulted(dsfRewards.CumulativeRewards.size());
	const FString& anyTag = UUScenUtil::GetAnyEntityTag();
	for (const auto& rewardPair : dsfRewards.CumulativeRewards)
	{
		auto& newItem = scenarioRewards.PropertyCountRewards[newItemIndex];
		newItem.Condition.EntityId = anyTag;
		newItem.Condition.TypeName = UUEUtil::ToFString(rewardPair.TypeName);
		ConvertPropertyCondition(rewardPair.Filter, newItem.Condition);
		newItem.Reward = rewardPair.Reward;
		newItemIndex++;
	}

	return scenarioRewards;
}


void AUnrealScenarioXpActor::SetScenarioRewards(FScenarioRewards ScenarioRewards)
{
	using namespace discenfw;
	using namespace discenfw::xp;
	StateRewardRules stateReward;
	// ResultReward
	stateReward.ResultReward[ActionResult::SUCCEEDED] = ScenarioRewards.SuccessReward;
	stateReward.ResultReward[ActionResult::FAILED] = ScenarioRewards.FailureReward;
	stateReward.ResultReward[ActionResult::DEADLOCK] = ScenarioRewards.DeadlockReward;
	stateReward.ResultReward[ActionResult::IN_PROGRESS] = ScenarioRewards.InProgressReward;

	// EntityPropertyRewards
	for (const FPropertyConditionReward& reward : ScenarioRewards.EntityPropertyRewards)
	{
		std::string id = UUEUtil::ToStdString(reward.Condition.EntityId);
		stateReward.EntityConditionRewards.push_back(
		{
			{id,{ToPropertyCondition(reward.Condition)}}, reward.Reward }
		);
	}

	// EntityRelationshipRewards
	for (const FRelationshipConditionReward& reward : ScenarioRewards.EntityRelationshipRewards)
	{
		std::string id = UUEUtil::ToStdString(reward.Condition.EntityId);
		stateReward.EntityConditionRewards.push_back(
		{
			{id, {}, {ToRelationshipCondition(reward.Condition)}}, reward.Reward }
		);
	}

	// CumulativeRewards
	for (const FPropertyConditionReward& reward : ScenarioRewards.PropertyCountRewards)
	{
		std::string typeName = UUEUtil::ToStdString(reward.Condition.TypeName);
		PropertyReward propReward(
			typeName,
			ToPropertyCondition(reward.Condition),
			reward.Reward
		);
		stateReward.CumulativeRewards.push_back(propReward);
	}

	DiScenFw()->SetRewardRules(AgentName, stateReward);
	NewEpisode();
}


FString AUnrealScenarioXpActor::GetEntityType(FString EntityId)
{
	using namespace discenfw;
	using namespace discenfw::xp;
	const std::shared_ptr<EnvironmentState> environmentState = DiScenFw()->GetLastScenarioState(AgentName);
	std::string id = UUEUtil::ToStdString(EntityId);
	if (environmentState && environmentState->ContainsEntity(id))
	{
		const std::string& entityType = environmentState->GetEntityState(id)->GetTypeName();
		return UUEUtil::ToFString(entityType);
	}
	return FString();
}


TArray<FString> AUnrealScenarioXpActor::GetEntityTypeDefaultProperties(FString EntityTypeId)
{
	if (!EntityTypeId.IsEmpty())
	{
		using namespace discenfw;
		using namespace discenfw::xp;
		const std::string entityTypeName = UUEUtil::ToStdString(EntityTypeId);
		std::shared_ptr<EntityStateType> entityStateType = GetLastModel()->GetEntityStateType(entityTypeName);
		if (entityStateType)
		{
			TArray<FString> propNames;
			const std::map<std::string, std::vector<std::string>>& possibleProperties =
				entityStateType->GetPossiblePropertyValues();
			for (const auto& prop : possibleProperties)
			{
				propNames.Add(UUEUtil::ToFString(prop.first));
			}
			return propNames;
		}
	}
	return TArray<FString>();
}


TArray<FString> AUnrealScenarioXpActor::GetEntityTypeDefaultPropertyValues(FString EntityTypeId, FString PropertyName)
{
	using namespace discenfw;
	using namespace discenfw::xp;
	const std::string entityTypeName = UUEUtil::ToStdString(EntityTypeId);
	std::shared_ptr<EntityStateType> entityStateType = GetLastModel()->GetEntityStateType(entityTypeName);
	if (entityStateType)
	{
		TArray<FString> propValues;
		const std::map<std::string, std::vector<std::string>>& possibleProperties =
			entityStateType->GetPossiblePropertyValues();
		const std::string entityPropName = UUEUtil::ToStdString(PropertyName);
		if (possibleProperties.find(entityPropName) != possibleProperties.cend())
		{
			for (const auto& val : possibleProperties.at(entityPropName))
			{
				propValues.Add(UUEUtil::ToFString(val));
			}
			return propValues;
		}
	}
	return TArray<FString>();
}


TArray<FString> AUnrealScenarioXpActor::GetPropertyChoices(FString EntityId, FString EntityTypeName)
{
	if (UUScenUtil::IsSpecialEntityTag(EntityId))
	{
		if (EntityTypeName.IsEmpty())
		{
			return UUScenUtil::GetAllDefinedPropertyNames();
		}
		return UUScenUtil::GetDefinedPropertyNames(EntityTypeName);
	}

	FString EntityTypeId = GetEntityType(EntityId);
	return GetEntityTypeDefaultProperties(EntityTypeId);
}


TArray<FString> AUnrealScenarioXpActor::GetPropertyValueChoices(FString EntityId, FString PropertyName, FString EntityTypeName)
{
	if (UUScenUtil::IsSpecialEntityTag(EntityId))
	{
		if (EntityTypeName.IsEmpty())
		{
			return UUScenUtil::GetAllDefinedPropertyValues(PropertyName);
		}
		return UUScenUtil::GetDefinedPropertyValues(PropertyName, EntityTypeName);
	}

	FString EntityTypeId = GetEntityType(EntityId);
	return GetEntityTypeDefaultPropertyValues(EntityTypeId, PropertyName);
}



FAgentStats AUnrealScenarioXpActor::GetAgentStatistics()
{
	InitXp();
	using namespace discenfw;
	using namespace discenfw::xp;
	xp::AgentStats stats = DiScenFw()->GetAgentStats(AgentName);
	Statistics.DeadlockCount = stats.DeadlockCount;
	Statistics.SuccessCount = stats.SuccessCount;
	Statistics.FailureCount = stats.FailedCount;
	Statistics.EpisodesCount = stats.EpisodesCount;
	Statistics.SuccessRate = 0;
	if (Statistics.SuccessCount == 0)
	{
		Statistics.BestPerformance = 0;
	}
	if (Statistics.EpisodesCount > 0)
	{
		Statistics.SuccessRate = Statistics.SuccessCount * 100 / Statistics.EpisodesCount;
	}
	return Statistics;
}


void AUnrealScenarioXpActor::ResetAgentStatistics()
{
	InitXp();
	using namespace discenfw;
	using namespace discenfw::xp;
	DiScenFw()->ResetAgentStats(AgentName);
	Statistics = FAgentStats();
}


void AUnrealScenarioXpActor::ResetAgent()
{
	InitXp();
	using namespace discenfw;
	using namespace discenfw::xp;
	DiScenFw()->ResetAgent(AgentName);
	Statistics = FAgentStats();
}

int AUnrealScenarioXpActor::GetLastScenarioStateReward()
{
	using namespace discenfw;
	using namespace discenfw::xp;
	ActionOutcome outcome = DiScenFw()->GetLastActionOutcome(AgentName);
	return outcome.Reward;
}


FRLConfig AUnrealScenarioXpActor::GetRLConfiguration()
{
	using namespace discenfw;
	using namespace discenfw::xp;
	std::shared_ptr<IAgentConfiguration> config = DiScenFw()->GetAgentConfiguration(AgentName);
	if (!config || !config->IsA("RLConfig"))
	{
		UUScenUtil::Log(Logger::ERROR, FString::Printf(TEXT("Invalid agent configuration for '%s'."), UTF8_TO_TCHAR(AgentName.c_str())));
		return FRLConfig();
	}
	std::shared_ptr<RLConfig> rlCfg = std::static_pointer_cast<RLConfig>(config);
	FRLConfig ueRLConfig;
	ueRLConfig.InitialValue = rlCfg->InitialValue;
	ueRLConfig.SampleAverage = rlCfg->SampleAverage;
	ueRLConfig.FixedStepSize = rlCfg->FixedStepSize;
	ueRLConfig.DiscountRate = rlCfg->DiscountRate;
	ueRLConfig.EpsilonGreedy = rlCfg->Epsilon;
	ueRLConfig.EpsilonReduction = rlCfg->EpsilonReduction;
	return ueRLConfig;
}


void AUnrealScenarioXpActor::SetRLConfiguration(FRLConfig RLConfiguration)
{
	using namespace discenfw::xp;
	std::shared_ptr<RLConfig> rlCfg = std::make_shared<RLConfig>();
	rlCfg->InitialValue = RLConfiguration.InitialValue;
	rlCfg->SampleAverage = RLConfiguration.SampleAverage;
	rlCfg->FixedStepSize = RLConfiguration.FixedStepSize;
	rlCfg->DiscountRate = RLConfiguration.DiscountRate;
	rlCfg->Epsilon = RLConfiguration.EpsilonGreedy;
	rlCfg->EpsilonReduction = RLConfiguration.EpsilonReduction;

	DiScenFw()->SetAgentConfiguration(AgentName, rlCfg);
}


bool AUnrealScenarioXpActor::LoadRLConfiguration()
{
	const std::string rlCfgFileNameStr = UUEUtil::ToStdString(ScenarioDir() + RLConfigFileName);
	using namespace discenfw;
	if (DiScenFw()->LoadRLConfiguration(AgentName, rlCfgFileNameStr))
	{
		UUScenUtil::Log(Logger::LOG, TEXT("Learning configuration loaded from ") + RLConfigFileName);
	}
	else
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Failed to load learning configuration from ") + ScenarioDir() + RLConfigFileName);
	}
	return false;
}


bool AUnrealScenarioXpActor::SaveRLConfiguration()
{
	const std::string rlCfgFileNameStr = UUEUtil::ToStdString(ScenarioDir() + RLConfigFileName);
	using namespace discenfw;
	if (DiScenFw()->SaveRLConfiguration(AgentName, rlCfgFileNameStr))
	{
		UUScenUtil::Log(Logger::LOG, TEXT("Learning configuration saved to ") + RLConfigFileName);
	}
	else
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Failed to save learning configuration to ") + ScenarioDir() + RLConfigFileName);
	}
	return false;
}


bool AUnrealScenarioXpActor::SetSystemFailureIgnored(bool bIgnored)
{
	using namespace discenfw;
	return DiScenFw()->SetSystemFailureIgnored(AgentName, bIgnored);
}


bool AUnrealScenarioXpActor::IsSystemFailureIgnored()
{
	using namespace discenfw;
	return DiScenFw()->IsSystemFailureIgnored(AgentName);
}



bool AUnrealScenarioXpActor::SetDiscountingConstant(float DiscountingConstant)
{
	using namespace discenfw;
	return DiScenFw()->SetDiscountingConstant(AgentName, DiscountingConstant);
}


float AUnrealScenarioXpActor::GetDiscountingConstant() const
{
	using namespace discenfw;
	return DiScenFw()->GetDiscountingConstant(AgentName);
}


void AUnrealScenarioXpActor::SetAutoTrainingLiveUpdate(bool bEnabled)
{
	if (!bAutoTrainingLiveUpdate)
	{
		SyncSceneState();
		UpdateHints();
	}
	bAutoTrainingLiveUpdate = bEnabled;
}

void AUnrealScenarioXpActor::SetAutoTraining(bool bEnabled, bool bStopOnCompletion, bool bStopOnSuccess)
{
	bool prevEnabled = bAutoTrainingEnabled;
	bAutoTrainingEnabled = bEnabled;
	bStopAutoTrainingOnCompletion = bStopOnCompletion;
	bAutoTrainingStopOnSuccess = bStopOnSuccess;
	NewEpisode();
}


void AUnrealScenarioXpActor::NewEpisode()
{
	CancelEditing();
	InitXp();
	using namespace discenfw;
	if (DiScenFw()->NewEpisode(AgentName))
	{
		SyncSceneState();
		UpdateHints();
	}
}


void AUnrealScenarioXpActor::TrainingStep()
{
	InitXp();
	using namespace discenfw;
	using namespace discenfw::xp;
	AgentMode mode = (bLearningEnabled&&bAutoTrainingLoopDetection) ? AgentMode::DEFAULT
		: bLearningEnabled ? AgentMode::LEARN
		: bAutoTrainingLoopDetection ? AgentMode::LOOP_DETECTION
		: AgentMode::JUST_ACT;

	bool liveUpdate = bAutoTrainingLiveUpdate;

	ActionOutcome outcome = DiScenFw()->Train(AgentName, bAutoTrainingUpdateXp, mode);
	if (bAutoTrainingEnabled && bStopAutoTrainingOnCompletion && outcome.CompletedEpisode)
	{
		bAutoTrainingEnabled = false;
		liveUpdate = true;
	}
	if (bAutoTrainingEnabled && bAutoTrainingStopOnSuccess && outcome.Result == ActionResult::SUCCEEDED)
	{
		bAutoTrainingEnabled = false;
		liveUpdate = true;
	}
	xp::AgentStats stats = DiScenFw()->GetAgentStats(AgentName);
	Statistics.DeadlockCount = stats.DeadlockCount;
	Statistics.SuccessCount = stats.SuccessCount;
	Statistics.FailureCount = stats.FailedCount;
	if(Statistics.BestPerformance < outcome.Performance)
	{
		Statistics.BestPerformance = outcome.Performance;
	}
	Logger::ELogLevel logLevel = Logger::LOG;
	FString resultMsg;
	if (outcome.Error != ActionError::NO_ERROR)
	{
		resultMsg = TEXT("ERROR");
		logLevel = Logger::ERROR;
	}
	else
	{
		resultMsg = UUEUtil::ToFString(ActionResultToString(outcome.Result));
		if (resultMsg.IsEmpty())
		{
			resultMsg = "(in progress)";
		}
		OnActionDone(outcome.CompletedEpisode, liveUpdate);
	}
	UUScenUtil::Log(logLevel, TEXT("Training result = ") + resultMsg, false, true, TEXT("TRAIN"));
}


void AUnrealScenarioXpActor::LoadExperience()
{
	CancelEditing();
	InitXp();
	using namespace discenfw;
	const std::string xpFileNameStr = UUEUtil::ToStdString(ScenarioDir() + XpFileName);
	if (DiScenFw()->LoadExperience(AgentName, xpFileNameStr))
	{
		UUScenUtil::Log(Logger::LOG, TEXT("Experience loaded from ") + XpFileName, false, true);
	}
	else
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Failed to load experience from ") + ScenarioDir() + XpFileName, false, true);
	}
	SyncSceneState();
	UpdateHints();
}


void AUnrealScenarioXpActor::SaveExperience()
{
	InitXp();
	using namespace discenfw;

	if (bOptimizeSavedXp)
	{
		DiScenFw()->OptimizeExperienceForAssistance(AgentName);
	}
	const std::string xpFileNameStr = UUEUtil::ToStdString(ScenarioDir() + XpFileName);
	if (DiScenFw()->SaveExperience(AgentName, xpFileNameStr))
	{
		UUScenUtil::Log(Logger::LOG, TEXT("Experience saved to ") + XpFileName);
	}
	else
	{
		UUScenUtil::Log(Logger::ERROR, TEXT("Failed to save experience to ") + ScenarioDir() + XpFileName);
	}
	if (bOptimizeSavedXp)
	{
		UpdateHints();
	}
}


void AUnrealScenarioXpActor::ClearExperience()
{
	CancelEditing();
	using namespace discenfw;
	using namespace discenfw::xp;
	if (DiScenFw()->ClearExperience(AgentName))
	{
		NewEpisode();
	}
}


void AUnrealScenarioXpActor::BeginPlay()
{
	Super::BeginPlay();

	NewEpisode();
}


void AUnrealScenarioXpActor::InitDigitalScenarioFramework()
{
	Super::InitDigitalScenarioFramework();
	InitXp();
}


void AUnrealScenarioXpActor::OnScenarioLoaded()
{
	using namespace discenfw;
	DiScenFw()->ConfigureSystemFromScenario();
	Statistics = FAgentStats();
	Super::OnScenarioLoaded();
	NewEpisode();
}


void AUnrealScenarioXpActor::OnEntityAdded()
{
	Super::OnEntityAdded();
	using namespace discenfw;
	//DiScenFw()->ConfigureSystemFromScenario();
	Statistics = FAgentStats();
	SyncSceneState();
	UpdateHints();
	//NewEpisode();
	// TODO: review the consequences of a new entity added
}


void AUnrealScenarioXpActor::OnEntityRemoved()
{
	using namespace discenfw;
	Super::OnEntityRemoved();
	SyncSceneState();
	UpdateHints();
	// TODO: review the consequences of a deletion
}


void AUnrealScenarioXpActor::InitCyberSystem()
{
	if (CyberSystemName.empty())
	{
		NotifyMissingImplementation();
		return;
	}
	using namespace discenfw;
	using namespace discenfw::xp;
	using namespace discenfw::ve;

#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 20
	FString projectPath = FPaths::ProjectDir();
#else
	FString projectPath = FPaths::GameDir();
#endif
	// TODO: digital system path is platform dependent
	const std::string cyberSystemPath = UUEUtil::ToStdString(projectPath) + "Binaries/Win64/" + CyberSystemName;
	if (!DiScenFw()->IsCyberSystemLoaded(cyberSystemPath))
	{
		//AgentName = "assistant";
		//GetGlobalLogger()->SetDefaultOutput(true, true);
		DiScenFw()->LoadCyberSystem(cyberSystemPath);
		DiScenFw()->ConfigureSystemFromScenario();
	}
}


void AUnrealScenarioXpActor::InitXp()
{
	if (!bXpInitialized)
	{
		using namespace discenfw;
		using namespace discenfw::xp;
		using namespace discenfw::ve;
		InitCyberSystem();
		DiScenFw()->CreateAgent(AgentName);

		DiScenFw()->SetCurrentGoal(AgentName, UUEUtil::ToStdString(CurrentGoalName));
		bXpInitialized = true;
	}
}


void AUnrealScenarioXpActor::TakeAction(discenfw::xp::Action ChosenAction)
{
	using namespace discenfw;
	using namespace discenfw::xp;
	ActionOutcome outcome;
	AgentMode mode = (bLearningEnabled&&bAutoTrainingLoopDetection) ? AgentMode::DEFAULT
		: bLearningEnabled ? AgentMode::LEARN
		: bAutoTrainingLoopDetection ? AgentMode::LOOP_DETECTION
		: AgentMode::JUST_ACT;
	outcome = DiScenFw()->TakeAction(AgentName, ChosenAction, bAutoTrainingUpdateXp, mode);
	if (bLearningEnabled)
	{
		xp::AgentStats stats = DiScenFw()->GetAgentStats(AgentName);
		Statistics.DeadlockCount = stats.DeadlockCount;
		Statistics.SuccessCount = stats.SuccessCount;
		Statistics.FailureCount = stats.FailedCount;
		if (Statistics.BestPerformance < outcome.Performance)
		{
			Statistics.BestPerformance = outcome.Performance;
		}
	}

	OnActionDone(outcome.CompletedEpisode);
}


void AUnrealScenarioXpActor::OnActionDone(bool bCompletedEpisode, bool bUpdateScene)
{
	using namespace discenfw;
	using namespace discenfw::xp;

	// in case of completed episode the assistant is automatically promoted (TRAINEE --> ASSISTANT)
	//if (bCompletedEpisode)
	//{
	//	DiScenFw()->SetLevel(AgentName, ExperienceLevel::ASSISTANT);
	//}
	if (bUpdateScene)
	{
		SyncSceneState();
		UpdateHints();
	}

}


void AUnrealScenarioXpActor::NotifyMissingImplementation()
{
	// TODO: maybe an interface could be defined for these methods
	UUScenUtil::Log(Logger::FATAL, TEXT("AUnrealScenarioXpActor cannot be used directly, derive and implement a new class, define CyberSystemName and override virtual methods."), true, true, "USCENXP_ABSTRACT");
}

