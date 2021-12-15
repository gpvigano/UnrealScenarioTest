// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "SimpleCircuitTestActor.h"

#include "UEUtil.h"
#include "UScenUtil.h"
#include "EntityStateComponent.h"

#include "discenfw/util/MessageLog.h"
#include "discenfw/DigitalScenarioFramework.h"
#include "discenfw/ve/VirtualEnvironmentAPI.h"

FName ASimpleCircuitTestActor::ConnectedLeadTag = TEXT("ConnectedLead");


ASimpleCircuitTestActor::ASimpleCircuitTestActor()
{
	CyberSystemName = "SimplECircuitCybSys";
	CurrentGoalName = TEXT("Simple circuit");

	CatalogComponent->Name = TEXT("Electronic Components");

	InitElementSpawnSlot = FVector(0.0f, -550.0f, 450.0f);
	ElementSpawnSlot = InitElementSpawnSlot;
}


void ASimpleCircuitTestActor::BeginPlay()
{
	Super::BeginPlay();

	const std::string xpFilePath = UUEUtil::ToStdString(ScenarioDir() + XpFileName);

	using namespace discenfw;
	using namespace discenfw::xp;

	// load the experience at startup
	if (!DiScenFw()->LoadExperience(AgentName, xpFilePath))
	{
		// ... or create, configure and save it

		DiScenFw()->SetAgentRole(AgentName, "Test");

		PropertyCondition connectedCond({ "connected","true" });
		PropertyCondition swOnCond({ "position","1" });
		PropertyCondition litUpCond({ "lit up","true" });
		Condition successCond({ { "LED1",{litUpCond} } });
		successCond.AddCondition(LogicOp::AND, Condition({ { "SW1",{connectedCond, swOnCond} } }));
		DiScenFw()->SetSuccessCondition(AgentName, successCond);

		StateRewardRules scenarioReward;
		scenarioReward.ResultReward =
		{
			{ ActionResult::IN_PROGRESS,-1 },
			{ ActionResult::SUCCEEDED,200 },
			{ ActionResult::FAILED,-200 },
			{ ActionResult::DEADLOCK,-10 }
		};
		scenarioReward.CumulativeRewards =
		{
			PropertyReward( "Electronic Component", { "connected","true" }, -5 )
		};
		scenarioReward.EntityConditionRewards =
		{
			{ {"R2",{{ "connected","true" }} }, -10 },
			{ {"LED1",{{ "connected","true" }} }, 10 },
		};
		DiScenFw()->SetRewardRules(AgentName, scenarioReward);

		//assistant->CurrentExperience()->ScenarioReward.ResultReward[ActionResult::IN_PROGRESS] = -1;
		//assistant->CurrentExperience()->ScenarioReward.ResultReward[ActionResult::SUCCEEDED] = 200;
		//assistant->CurrentExperience()->ScenarioReward.ResultReward[ActionResult::FAILED] = -200;

		//assistant->CurrentExperience()->ScenarioReward.PropertyCountRewards =
		//{
		//	{ { "connected","true" }, -5 },
		//};

		//assistant->CurrentExperience()->ScenarioReward.EntityConditionRewards =
		//{
		//	{ "R2",{ { "connected","true" }, -10 } },
		//	{ "LED1",{ { "connected","true" }, 10 } },
		//};
		DiScenFw()->SetLogEnabled(AgentName, true);
		//DiScenFw()->SetLevel(AgentName, ExperienceLevel::TRAINEE);
		//DiScenFw()->SetLevel(AgentName, ExperienceLevel::ASSISTANT);

		DiScenFw()->SaveExperience(AgentName, xpFilePath);
	}
}


void ASimpleCircuitTestActor::OnEntityClicked(AActor* ClickedEntity, UPrimitiveComponent* ClickedComp)
{
	if (ClickedComp->ComponentHasTag("SwitchCursor"))
	{
		bTemporaryLockedSelection = true;
		SwitchSwitch(ClickedEntity);
	}
	else if (ClickedComp->ComponentHasTag(UUScenUtil::SocketTag))
	{
		if (ClickedComp->ComponentHasTag(ConnectedLeadTag))
		{
			DisconnectLead(ClickedEntity, ClickedComp);
		}
		else
		{
			ConnectLead(ClickedEntity, ClickedComp);
		}
	}
	else
	{
		Super::OnEntityClicked(ClickedEntity, ClickedComp);
	}
}


void ASimpleCircuitTestActor::OnStartMovingEntity(AActor* MovedActor)
{
	MovingCables.Reset();
	if (MovedActor)
	{
		TArray<USceneComponent*> sceneComponents;
		UUEUtil::GetSceneComponents(MovedActor, sceneComponents);
		for (AActor* cableActor : Cables)
		{
			USceneComponent* startLead = UUEUtil::GetActorSceneCompProperty(cableActor,FName("StartLead"));
			USceneComponent* endLead = UUEUtil::GetActorSceneCompProperty(cableActor,FName("EndLead"));

			if (sceneComponents.Contains(startLead) || sceneComponents.Contains(endLead))
			{
				MovingCables.AddUnique(cableActor);
			}
		}
	}
}


void ASimpleCircuitTestActor::OnEntityMoved(AActor* MovedActor)
{
	TooltipLabel->SetActorLocation(MovedActor->GetActorLocation());
	for (AActor* cableActor : MovingCables)
	{
		UUEUtil::CallActorFunctionWithArguments(cableActor, TEXT("UpdateCable"));
		//FOutputDeviceNull ar;
		//const FString command = FString::Printf(TEXT("UpdateCable"));
		//cableActor->CallFunctionByNameWithArguments(*command, ar, NULL, true);
	}
}


void ASimpleCircuitTestActor::OnStopMovingEntity(AActor* MovedActor)
{
	OnEntityMoved(MovedActor);
	MovingCables.Reset();
}


void ASimpleCircuitTestActor::OnBeginCursorOverEntity(UPrimitiveComponent* TouchedComponent)
{
	Super::OnBeginCursorOverEntity(TouchedComponent);

	if (TouchedComponent->ComponentHasTag(UUScenUtil::SocketTag))
	{
		SetLabelText(TouchedComponent->GetName(), TouchedComponent->GetComponentLocation());
	}
	else
	{
		AActor* entityActor = GetEntityFromActor(TouchedComponent->GetOwner());
		UEntityDataComponent* entityComponent = UUScenUtil::GetEntityData(entityActor);
		if (entityComponent)
		{
			SetLabelText(entityComponent->Identifier, entityActor->GetActorLocation());
		}
	}
}


void ASimpleCircuitTestActor::OnEndCursorOverEntity(UPrimitiveComponent* TouchedComponent)
{
	if (MovingEntityActor)
	{
		return;
	}

	Super::OnEndCursorOverEntity(TouchedComponent);
	UUEUtil::SetActorActive(TooltipLabel, false);
}


//void ASimpleCircuitTestActor::OnCableClicked(UPrimitiveComponent* ClickedComp, FKey ButtonPressed)
//{
//	USceneComponent* startLead = UUEUtil::GetActorSceneCompProperty(ClickedComp->GetOwner(), TEXT("StartLead"));
//	USceneComponent* endLead = UUEUtil::GetActorSceneCompProperty(ClickedComp->GetOwner(), TEXT("EndLead"));
//	if(startLead && endLead)
//	{
//		UUEUtil::Log.LogMessage(Logger::LOG, FString::Printf(TEXT("disconnect %s-%s"),
//			*(startLead->GetName()), *(endLead->GetName())), "UnrealScenarioTest");
//	}
//}


bool ASimpleCircuitTestActor::IsEditing()
{
	return Super::IsEditing() || NewCableActor != nullptr;
}


void ASimpleCircuitTestActor::UpdateEditing(float DeltaTime)
{
	Super::UpdateEditing(DeltaTime);

	if (NewCableActor)
	{
		// update the cable during interactive components connection

		APlayerController* playerController = GetWorld()->GetFirstPlayerController();
		FVector dragPos;
		//FVector newCableStartPos = StartLeadComp->GetComponentLocation();
		//UUEUtil::SetActorVectorProperty(NewCableActor, TEXT("StartPos"), newCableStartPos);
		if (UUEUtil::MouseDragLocation(playerController, NewCableEndDistance, dragPos))
		{
			UUEUtil::SetActorVectorProperty(NewCableActor, TEXT("EndPos"), dragPos);
			UUEUtil::CallActorFunctionWithArguments(NewCableActor, TEXT("UpdateCable"));
		}
	}

}


void ASimpleCircuitTestActor::CancelEditing()
{
	Super::CancelEditing();
	if (NewCableActor)
	{
		UUEUtil::SetActorActive(NewCableActor, false);
		StopWiring();
		UpdateHints();
	}
}


AActor* ASimpleCircuitTestActor::GetOrCreateCable(int CableIndex)
{
	if (CableIndex < 0)
	{
		CableIndex = NumCables;
	}
	while (Cables.Num() <= CableIndex)
	{
		FString cableName = TEXT("cable") + FString::FormatAsNumber(CableIndex);
		//AActor* cableActor = UUEUtil::CreateCloneOfActor(CableTemplate, cableName, ScenarioActor);
		AActor* cableActor = AssetIndexComponent->CreateActorFromAsset(CableTemplateName);

		//TArray<UMeshComponent*> meshComponents;
		//UUEUtil::GetMeshComponents(cableActor, meshComponents);
		//for (int i = 0; i < meshComponents.Num(); i++)
		//{
		//	UMeshComponent* meshComponent = meshComponents[i];
		//	if (meshComponent && !meshComponent->OnClicked.IsBound())
		//	{
		//		meshComponent->OnClicked.AddDynamic(this, &ASimpleCircuitTestActor::OnCableClicked);
		//		meshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
		//		meshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		//	}
		//}

		Cables.Add(cableActor);
	}
	UUEUtil::SetActorActive(Cables[CableIndex], true);
	return Cables[CableIndex];
}



void ASimpleCircuitTestActor::StopWiring()
{
	if (NewCableActor)
	{
		//	UUEUtil::SetActorActive(NewCableActor, false);
		UUEUtil::CallActorFunctionWithArguments(NewCableActor, TEXT("Disconnect"));
	}
	StartLeadComp = nullptr;
	NewCableActor = nullptr;
}


void ASimpleCircuitTestActor::SwitchSwitch(AActor* ClickedEntity)
{
	UEntityDataComponent* entityComponent = CastChecked<UEntityDataComponent>(ClickedEntity->GetComponentByClass(UEntityDataComponent::StaticClass()));
	if (entityComponent->Type == TEXT("Switch"))
	{
		StopWiring();
		//USceneComponent* cursor = UUEUtil::GetActorSceneCompProperty(ClickedEntity, FName("SwitchCursor"));
		UEntityStateComponent* entityStateComponent = CastChecked<UEntityStateComponent>(
			ClickedEntity->GetComponentByClass(UEntityStateComponent::StaticClass()));
		using namespace discenfw;
		using namespace discenfw::xp;
		bool swOn = entityStateComponent->CheckProperty("position", "1");
		UUScenUtil::Log(Logger::LOG, swOn ? TEXT("USER SWITCH OFF") : TEXT("USER SWITCH ON"), false, true);

		const Action action("switch", { UUEUtil::ToStdString(entityComponent->Identifier), swOn ? "0" : "1" });
		TakeAction(action);
	}
}


void ASimpleCircuitTestActor::ConnectLead(AActor* ClickedEntity, UPrimitiveComponent* ClickedComp)
{
	UElementDataComponent* elemComp = CastChecked<UElementDataComponent>(ClickedEntity->GetComponentByClass(UElementDataComponent::StaticClass()));

	if (StartLeadComp && StartLeadComp != ClickedComp)
	{
		UEntityDataComponent* endEntityComp = CastChecked<UEntityDataComponent>(ClickedEntity->GetComponentByClass(UEntityDataComponent::StaticClass()));
		UEntityDataComponent* startEntityComp = Cast<UEntityDataComponent>(StartLeadComp->GetOwner()->GetComponentByClass(UEntityDataComponent::StaticClass()));
		const std::string startComp = UUEUtil::ToStdString(startEntityComp->Identifier);
		const std::string startLead = UUEUtil::ToStdString(StartLeadComp->GetName());
		const std::string endComp = UUEUtil::ToStdString(endEntityComp->Identifier);
		const std::string endLead = UUEUtil::ToStdString(ClickedComp->GetName());
		//StopWiring();
		NumCables++;
		NewCableActor = nullptr;
		StartLeadComp = nullptr;
		InitXp();
		using namespace discenfw;
		using namespace discenfw::xp;

		TakeAction({ "connect",{ startComp, startLead, endComp, endLead } });
	}
	else
	{
		StopWiring();
		StartLeadComp = ClickedComp;
		NewCableActor = GetOrCreateCable();
		FVector newCableStartPos = StartLeadComp->GetComponentLocation();
		UUEUtil::SetActorSceneCompProperty(NewCableActor, TEXT("StartLead"), StartLeadComp);
		//UUEUtil::SetActorSceneCompProperty(NewCableActor, TEXT("EndLead"), nullptr);
		//UUEUtil::SetActorVectorProperty(NewCableActor, TEXT("StartPos"), newCableStartPos);

		NewCableEndDistance = UUEUtil::GetDistanceFromEyesViewPoint(NewCableActor);
		FVector dragPos;
		APlayerController* playerController = GetWorld()->GetFirstPlayerController();
		if (UUEUtil::MouseDragLocation(playerController, NewCableEndDistance, dragPos))
		{
			UUEUtil::SetActorVectorProperty(NewCableActor, TEXT("EndPos"), dragPos);
		}
		UUEUtil::CallActorFunctionWithArguments(NewCableActor, TEXT("UpdateCable"));
		UpdateHints();
	}
}


void ASimpleCircuitTestActor::DisconnectLead(AActor* ClickedEntity, UPrimitiveComponent* ClickedComp)
{
	UElementDataComponent* entityComp = Cast<UElementDataComponent>(ClickedEntity->GetComponentByClass(UElementDataComponent::StaticClass()));
	const std::string compId = UUEUtil::ToStdString(entityComp->Identifier);
	const std::string leadId = UUEUtil::ToStdString(ClickedComp->GetName());
	using namespace discenfw;
	using namespace discenfw::xp;
	TakeAction({ "disconnect",{ compId, leadId } });
}


void ASimpleCircuitTestActor::SyncEntityState(
	const std::shared_ptr<discenfw::Entity> ScenEntity,
	const std::shared_ptr<discenfw::xp::EntityState> XpEntityState,
	UEntityStateComponent* compState)
{
	const std::string& entityId = ScenEntity->GetIdentifier();
	FString idString = UUEUtil::ToFString(entityId);
	FString msg = UUEUtil::ToFString(ScenEntity->GetIdentity().Type + " " + entityId + "-->");

	bool burntOut = XpEntityState->HasPropertySet("burnt out", "true");
	FString burntOutVal = burntOut ? TEXT("true") : TEXT("false");
	bool burntOutChanged = !compState->CheckProperty(TEXT("burnt out"), burntOutVal);
	if (burntOutChanged)
	{
		compState->ApplyProperty(TEXT("burnt out"), burntOutVal);
		if (burntOut)
		{
			UUScenUtil::Log(Logger::DEBUG, msg + TEXT("BURNT OUT"), false, true, idString);
		}
	}

	if (ScenEntity->GetIdentity().Type == "LED")
	{
		bool litUp = XpEntityState->HasPropertySet("lit up", "true");
		FString propVal = litUp ? TEXT("true") : TEXT("false");
		//bool changed = !compState->CheckProperty(TEXT("lit up"), propVal);
		//if (changed)
		//{
		//	UUScenUtil::Log(Logger::DEBUG, litUp ? msg + TEXT("LIT UP") : msg + TEXT("OFF"), false, true, idString + "LED_LITUP");
		//}
		compState->ApplyProperty(TEXT("lit up"), propVal);
	}

	if (ScenEntity->GetIdentity().Type == "Switch")
	{
		bool switchOn = XpEntityState->HasPropertySet("position", "1");
		FString propVal = switchOn ? TEXT("1") : TEXT("0");
		//bool changed = !compState->CheckProperty(TEXT("position"), propVal);
		//if (changed)
		//{
		//	UUScenUtil::Log(Logger::DEBUG, msg + (switchOn ? TEXT("ON") : TEXT("OFF")), false, true, idString + "SWITCH_ON");
		//}
		compState->ApplyProperty(TEXT("position"), propVal);
	}
}


void ASimpleCircuitTestActor::SyncRelationships(
	const std::shared_ptr<discenfw::xp::EnvironmentState> XpScenarioState)
{
	typedef std::pair<std::string, std::string> LeadInfo;
	std::set< std::pair<LeadInfo, LeadInfo> > connections;
	for (const auto& entityState : XpScenarioState->GetEntityStates())
	{
		for (const auto& rel : entityState.second->GetRelationships())
		{
			std::string firstEnt = entityState.first;
			std::string firstLead = rel.first;
			std::string secondEnt = rel.second.EntityId;
			std::string secondLead = rel.second.LinkId;
			if (entityState.first > rel.second.EntityId)
			{
				std::swap(firstEnt, secondEnt);
				std::swap(firstLead, secondLead);
			}
			connections.insert(
			{
				{ firstEnt, firstLead },
				{ secondEnt, secondLead }
			});
		}
	}

	for (auto& entityActor : EntityActors)
	{
		TArray<USceneComponent*> sceneComponents;
		UUEUtil::GetSceneComponents(entityActor, sceneComponents);
		for (auto& sceneComp : sceneComponents)
		{
			sceneComp->ComponentTags.Remove(ConnectedLeadTag);
		}
	}

	FString msg;
	int cableCount = 0;
	for (const auto& conn : connections)
	{
		FString comp1 = UUEUtil::ToFString(conn.first.first);
		FString lead1 = UUEUtil::ToFString(conn.first.second);
		FString comp2 = UUEUtil::ToFString(conn.second.first);
		FString lead2 = UUEUtil::ToFString(conn.second.second);
		FString connStr = FString::Printf(TEXT("%s.%s - %s.%s"),
			*comp1, *lead1,
			*comp2, *lead2
			);
		msg.Append(connStr);
		msg.Append("  ");
		AActor* compActor1 = GetEntityActorFromId(conn.first.first);
		AActor* compActor2 = GetEntityActorFromId(conn.second.first);

		UElementDataComponent* elemComp1 = CastChecked<UElementDataComponent>(compActor1->GetComponentByClass(UElementDataComponent::StaticClass()));
		USceneComponent* leadComp1 = nullptr;
		if (elemComp1)
		{
			leadComp1 = elemComp1->GetSocketComponent(lead1);
		}
		if (!leadComp1)
		{
			leadComp1 = UUEUtil::FindSceneComponent(compActor1, lead1);
		}

		UElementDataComponent* elemComp2 = CastChecked<UElementDataComponent>(compActor2->GetComponentByClass(UElementDataComponent::StaticClass()));
		USceneComponent* leadComp2 = nullptr;
		if (elemComp2)
		{
			leadComp2 = elemComp2->GetSocketComponent(lead2);
		}
		if (!leadComp2)
		{
			leadComp2 = UUEUtil::FindSceneComponent(compActor2, lead2);
		}

		if (leadComp1&&leadComp2)
		{
			// create or reuse a cable to connect components leads
			AActor* cableActor = GetOrCreateCable(cableCount);
			const FString command = FString::Printf(TEXT("ConnectComponentLeads %s %s"), *(leadComp1->GetPathName()), *(leadComp2->GetPathName()));
			UUEUtil::CallActorFunctionWithArguments(cableActor, command);
			cableCount++;
			leadComp1->ComponentTags.AddUnique(ConnectedLeadTag);
			leadComp2->ComponentTags.AddUnique(ConnectedLeadTag);
		}
		else
		{
			if (leadComp1 == nullptr)
			{
				UUEUtil::Log.LogMessage(Logger::ERROR, FString::Printf(TEXT("Lead %s.%s not found"), *comp1, *lead1), "UnrealScenarioTest");
			}
			if (leadComp2 == nullptr)
			{
				UUEUtil::Log.LogMessage(Logger::ERROR, FString::Printf(TEXT("Lead %s.%s not found"), *comp2, *lead2), "UnrealScenarioTest");
			}
		}
	}

	// deactivate unused cables
	for (int i = cableCount; i < Cables.Num(); i++)
	{
		UUEUtil::CallActorFunctionWithArguments(Cables[i], TEXT("Disconnect"));
		UUEUtil::SetActorActive(Cables[i], false);
	}
	NumCables = cableCount;


	// Update cables attached to the moved entity according to the new state
	OnStartMovingEntity(MovingEntityActor);

}



void ASimpleCircuitTestActor::UpdateHints()
{
	Super::UpdateHints();

	using namespace discenfw;
	using namespace discenfw::xp;

	ActionOutcome episodeOutcome = DiScenFw()->GetLastActionOutcome(AgentName);


	UEntityDataComponent* startEntity = nullptr;
	if (StartLeadComp)
	{
		startEntity = Cast<UEntityDataComponent>(StartLeadComp->GetOwner()->GetComponentByClass(UEntityDataComponent::StaticClass()));
	}


	if (!IsEditing() && episodeOutcome.CompletedEpisode)
	{
		// get the last action sequence
		const std::vector<Action>& actionsSequence = DiScenFw()->GetActionsSequence(AgentName);
		UMaterialInterface* iconMaterial = nullptr;
		if (episodeOutcome.Result == ActionResult::SUCCEEDED)
		{
			// Show the final successful action sequence
			iconMaterial = IconMaterialSuggested;
		}
		else if (episodeOutcome.Result == ActionResult::FAILED)
		{
			// Show the last action that led to a failure
			iconMaterial = IconMaterialForbidden;
		}
		if (iconMaterial)
		{
			if (episodeOutcome.Result == ActionResult::FAILED)
			{
				UpdateActionHint(actionsSequence.back(), iconMaterial, startEntity);
			}
			else
			{
				UpdateActionsHints(actionsSequence, iconMaterial, startEntity);
			}
		}
		// TODO: what about hints related to a deadlock?
	}
	else
	{
		// Collect suggestions
		// Note: DiScenFw wrapper is needed to avoid passing vectors defined locally and allocated by DiScenFw

		const std::vector<Action>& suggestedActions = DiScenFw()->GetSuggestedActions(AgentName);
		const std::vector<Action>& forbiddenActions = DiScenFw()->GetForbiddenActions(AgentName);
		//const std::vector<Action> availableActions = DiScenFw()->GetAvailableActions();

		// update hints

		UpdateActionsHints(suggestedActions, IconMaterialSuggested, startEntity);
		UpdateActionsHints(forbiddenActions, IconMaterialForbidden, startEntity, startEntity != nullptr);
	}
}


#if WITH_EDITOR
void ASimpleCircuitTestActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (AssetIndexComponent && !AssetIndexComponent->IsActorBlueprintDefined(CableTemplateName))
	{
		UUScenUtil::Log(Logger::ERROR, CableTemplateName + TEXT(" actor blueprint not defined in Asset Index Component"));
	}
}
#endif


void ASimpleCircuitTestActor::UpdateActionsHints(
	const std::vector<discenfw::xp::Action>& ActionList,
	UMaterialInterface* IconMaterial,
	UEntityDataComponent* StartEntity,
	bool IncludeConnections)
{
	using namespace discenfw;
	using namespace discenfw::xp;
	for (const Action& action : ActionList)
	{
		UpdateActionHint(action, IconMaterial, StartEntity, IncludeConnections);
	}
}


void ASimpleCircuitTestActor::UpdateActionHint(
	const discenfw::xp::Action& WhichAction,
	UMaterialInterface* IconMaterial,
	UEntityDataComponent* StartEntity,
	bool IncludeConnections)
{
	// TODO: maybe a search based on the component name is not the smartest idea
	// All the lead icon components have the "Icon" tag, but the proper component must be found anyway.
	const FString iconSuffix = "_Icon";
	if (WhichAction.TypeId == "connect")
	{
		if (IncludeConnections)
		{
			// read action parameters
			FString comp1 = UUEUtil::ToFString(WhichAction.Params[0]);
			FString lead1 = UUEUtil::ToFString(WhichAction.Params[1]);
			FString comp2 = UUEUtil::ToFString(WhichAction.Params[2]);
			FString lead2 = UUEUtil::ToFString(WhichAction.Params[3]);

			// find in scene references
			AActor* compActor1 = GetEntityActorFromId(comp1);
			AActor* compActor2 = GetEntityActorFromId(comp2);
			UPrimitiveComponent* leadIcon1 = Cast<UPrimitiveComponent>(UUEUtil::FindSceneComponent(compActor1, lead1 + iconSuffix));
			UPrimitiveComponent* leadIcon2 = Cast<UPrimitiveComponent>(UUEUtil::FindSceneComponent(compActor2, lead2 + iconSuffix));

			// determine if one of them is the currently connected lead
			bool connectingLeads = StartEntity && StartLeadComp;
			bool sameLeadAs1 = connectingLeads && comp1 == StartEntity->Identifier && lead1 == StartLeadComp->GetName();
			bool sameLeadAs2 = connectingLeads && comp2 == StartEntity->Identifier && lead2 == StartLeadComp->GetName();

			// choose which must be rendered (hints are useful for not yet connected leads)
			bool toBeRendered1 = sameLeadAs2 || !connectingLeads;
			bool toBeRendered2 = sameLeadAs1 || !connectingLeads;
			bool toBeRendered = toBeRendered1 || toBeRendered2;

			// show proper icon(s)
			if (leadIcon1 && leadIcon2 && toBeRendered)
			{
				if (toBeRendered1)
				{
					UUEUtil::SetPrimMaterial(leadIcon1, IconMaterial);
					leadIcon1->bHiddenInGame = false;
				}
				if (toBeRendered2)
				{
					UUEUtil::SetPrimMaterial(leadIcon2, IconMaterial);
					leadIcon2->bHiddenInGame = false;
				}
			}
		}
	}
	else if (WhichAction.TypeId == "switch")
	{
		// read action parameters
		FString swId = UUEUtil::ToFString(WhichAction.Params[0]);
		FString swPos = UUEUtil::ToFString(WhichAction.Params[1]);

		// find in scene references
		AActor* swActor = GetEntityActorFromId(swId);
		UPrimitiveComponent* swIcon = Cast<UPrimitiveComponent>(UUEUtil::FindSceneComponent(swActor, TEXT("SwitchCursor") + iconSuffix));

		// show proper icon
		if (swIcon)
		{
			UPrimitiveComponent* primComp = Cast<UPrimitiveComponent>(swIcon);

			if (primComp)
			{
				UUEUtil::SetPrimMaterial(primComp, IconMaterial);
				//primComp->bRenderInMainPass = true;
				primComp->bHiddenInGame = false;
			}
		}
	}

}



