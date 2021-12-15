// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "UScenUtil.h"
#include "UEUtil.h"

#include <Runtime/Engine/Public/EngineUtils.h> // TActorIterator

#include <discenfw/DigitalScenarioFramework.h>
#include <discenfw/xp/Experience.h>
#include <discenfw/xp/EnvironmentModel.h>

FName UUScenUtil::SocketTag = TEXT("ElementSocket");

FName UUScenUtil::GizmoTag = TEXT("Gizmo");

FName UUScenUtil::OverlappingTag = TEXT("Overlapping");


AUnrealScenarioActor* UUScenUtil::FindUnrealScenario(UWorld* World)
{
	for (TActorIterator<AUnrealScenarioActor> actorItr(World); actorItr; ++actorItr)
	{
		if (actorItr)
		{
			return *actorItr;
		}
	}
	return nullptr;
}


FString UUScenUtil::GetAllEntitiesTag()
{
	using namespace discenfw::xp;
	return UUEUtil::ToFString(EntityCondition::ALL);
}


FString UUScenUtil::GetAnyEntityTag()
{
	using namespace discenfw::xp;
	return UUEUtil::ToFString(EntityCondition::ANY);
}


bool UUScenUtil::IsSpecialEntityTag(const FString& Tag)
{
	using namespace discenfw::xp;
	std::string tag = UUEUtil::ToStdString(Tag);
	return tag == EntityCondition::ALL || tag == EntityCondition::ANY;
}


FString UUScenUtil::GetEntityStateType(FString EntityId)
{
	using namespace discenfw;
	using namespace discenfw::xp;
	const std::shared_ptr<EnvironmentState> environmentState = DiScenFw()->GetLastSystemState();
	std::string id = UUEUtil::ToStdString(EntityId);
	if (environmentState && environmentState->ContainsEntity(id))
	{
		const std::string& entityType = environmentState->GetEntityState(id)->GetTypeName();
		return UUEUtil::ToFString(entityType);
	}
	return FString();
}


TArray<FString> UUScenUtil::GetEntityStateTypeNames()
{
	using namespace discenfw::xp;
	const std::vector<std::string>& typeNames = GetLastModel()->GetEntityStateTypeNames();
	TArray<FString> entityStateTypeNames;
	for (const std::string& typeName : typeNames)
	{
		entityStateTypeNames.Add(UUEUtil::ToFString(typeName));
	}
	return entityStateTypeNames;
}


TArray<FString> UUScenUtil::GetAllDefinedPropertyNames()
{
	using namespace discenfw::xp;
	const std::vector< std::shared_ptr<EntityStateType> >& stateTypes = GetLastModel()->GetEntityStateTypes();
	TArray<FString> allDefinedPropertyNames;
	for (const auto& stateType : stateTypes)
	{
		const auto& props = stateType->GetDefaultPropertyValues();
		for (const auto& propPair : props)
		{
			allDefinedPropertyNames.AddUnique(UUEUtil::ToFString(propPair.first));
		}
	}
	return allDefinedPropertyNames;
}


TArray<FString> UUScenUtil::GetDefinedPropertyNames(const FString& TypeName)
{
	using namespace discenfw::xp;
	const std::shared_ptr<EntityStateType> stateType =
		GetLastModel()->GetEntityStateType(UUEUtil::ToStdString(TypeName));
	TArray<FString> definedPropertyNames;
	const auto& props = stateType->GetDefaultPropertyValues();
	for (const auto& propPair : props)
	{
		definedPropertyNames.AddUnique(UUEUtil::ToFString(propPair.first));
	}
	return definedPropertyNames;
}


TArray<FString> UUScenUtil::GetAllDefinedPropertyValues(const FString& PropertyName)
{
	std::string propName = UUEUtil::ToStdString(PropertyName);
	using namespace discenfw::xp;
	const std::vector< std::shared_ptr<EntityStateType> > stateTypes = GetLastModel()->GetEntityStateTypes();
	TArray<FString> definedPropertyValues;
	for (const auto& stateType : stateTypes)
	{
		const auto& propValues = stateType->GetPossiblePropertyValues();
		for (const auto& propValuePair : propValues)
		{
			if (propValuePair.first == propName)
			{
				for (const std::string& val : propValuePair.second)
				{
					definedPropertyValues.AddUnique(UUEUtil::ToFString(val));
				}
			}
		}
	}
	return definedPropertyValues;
}


TArray<FString> UUScenUtil::GetDefinedPropertyValues(const FString& PropertyName, const FString& TypeName)
{
	TArray<FString> definedPropertyValues;
	using namespace discenfw::xp;
	const std::shared_ptr<EntityStateType> stateType =
		GetLastModel()->GetEntityStateType(UUEUtil::ToStdString(TypeName));
	if (!stateType)
	{
		return definedPropertyValues;
	}
	std::string propName = UUEUtil::ToStdString(PropertyName);
	const auto& propValues = stateType->GetPossiblePropertyValues();
	for (const auto& propValuePair : propValues)
	{
		if (propValuePair.first == propName)
		{
			for (const std::string& val : propValuePair.second)
			{
				definedPropertyValues.AddUnique(UUEUtil::ToFString(val));
			}
		}
	}
	return definedPropertyValues;
}


TArray<FString> UUScenUtil::GetEntityTypeLinks(const FString& TypeName)
{
	TArray<FString> entityTypeLinks;
	using namespace discenfw::xp;
	const std::shared_ptr<EntityStateType> stateType =
		GetLastModel()->GetEntityStateType(UUEUtil::ToStdString(TypeName));
	if (!stateType)
	{
		return entityTypeLinks;
	}
	const auto& links = stateType->GetLinks();
	for (const std::string& link : links)
	{
		entityTypeLinks.Add(UUEUtil::ToFString(link));
	}
	return entityTypeLinks;
}


TArray<FString> UUScenUtil::GetEntityStateLinks(const FString& EntityStateId)
{
	FString entityStateType = GetEntityStateType(EntityStateId);
	return GetEntityTypeLinks(entityStateType);
}


float UUScenUtil::ComputeDiscountingOrGainConstant(int SingleStepReward, int EpisodeReward)
{
	return discenfw::xp::ComputeDiscountingOrGainConstant(SingleStepReward, EpisodeReward);
}


UEntityDataComponent* UUScenUtil::GetEntityData(AActor* EntityActor)
{
	if (EntityActor == nullptr)
	{
		return nullptr;
	}
	UEntityDataComponent* entityComponent = CastChecked<UEntityDataComponent>(
		EntityActor->GetComponentByClass(UEntityDataComponent::StaticClass()),ECastCheckedType::NullAllowed);
	return entityComponent;
}


UElementDataComponent* UUScenUtil::GetElementData(AActor* EntityActor)
{
	if (EntityActor == nullptr)
	{
		return nullptr;
	}
	UElementDataComponent* elementComponent = CastChecked<UElementDataComponent>(
		EntityActor->GetComponentByClass(UElementDataComponent::StaticClass()),ECastCheckedType::NullAllowed);
	return elementComponent;
}


UConnectionElementComponent* UUScenUtil::GetConnectionElementData(AActor* EntityActor)
{
	if (EntityActor == nullptr)
	{
		return nullptr;
	}
	UConnectionElementComponent* elementComponent = CastChecked<UConnectionElementComponent>(
		EntityActor->GetComponentByClass(UConnectionElementComponent::StaticClass()),ECastCheckedType::NullAllowed);
	return elementComponent;
}


FString UUScenUtil::GetEntityIdentifier(AActor* EntityActor)
{
	UEntityDataComponent* entityComponent = GetEntityData(EntityActor);
	if (entityComponent == nullptr)
	{
		return FString();
	}
	return entityComponent->Identifier;
}


AActor* UUScenUtil::GetEntityFromActor(AActor* Actor)
{
	AActor* selectedEntityActor = Actor;
	AActor* entityActor = nullptr;
	UEntityDataComponent* entityComponent = nullptr;
	if (selectedEntityActor)
	{
		entityActor = Actor;
		entityComponent = GetEntityData(selectedEntityActor);
		while (entityComponent == nullptr && selectedEntityActor->GetAttachParentActor())
		{
			selectedEntityActor = selectedEntityActor->GetAttachParentActor();
			entityComponent = GetEntityData(selectedEntityActor);
			entityActor = selectedEntityActor;
		}
	}
	return entityActor;
}


FVector UUScenUtil::GetAnchorWorldPosition(const FConnectionAnchor& Anchor)
{
	FVector absPos;
	if (::IsValid(Anchor.Component))
	{
		absPos = Anchor.Component->GetComponentTransform().TransformPosition(Anchor.Offset);
	}
	else
	{
		absPos = Anchor.Offset;
	}
	return absPos;
}


FVector UUScenUtil::GetAnchorLocalPosition(const FConnectionAnchor& Anchor, USceneComponent* ReferenceComponent)
{
	FVector absPos = GetAnchorWorldPosition(Anchor);
	FVector localPos;
	if (::IsValid(ReferenceComponent))
	{
		localPos = ReferenceComponent->GetComponentTransform().InverseTransformPosition(absPos);
	}
	else
	{
		localPos = absPos;
	}
	return localPos;
}


void UUScenUtil::SetAnchorRelativePosition(FConnectionAnchor& Anchor, const FVector& Position, USceneComponent* ReferenceComponent)
{
	FVector absPos;
	if (::IsValid(ReferenceComponent))
	{
		absPos = ReferenceComponent->GetComponentTransform().TransformPosition(Position);
	}
	else
	{
		absPos = Position;
	}
	FVector localPos;
	if (::IsValid(Anchor.Component))
	{
		localPos = Anchor.Component->GetComponentTransform().InverseTransformPosition(absPos);
	}
	else
	{
		localPos = absPos;
	}
	Anchor.Offset = localPos;
}


bool UUScenUtil::AreSocketsCompatible(const FSocketInfo& ConnectingSocket, const FSocketInfo& AcceptingSocket)
{
	const FString* found = AcceptingSocket.Compatibility.FindByPredicate(
		[&ConnectingSocket](const FString& compatibleType)
	{
		return compatibleType == ConnectingSocket.Type;
	});

	return found!=nullptr;
}


void UUScenUtil::Log(Logger::ELogLevel Level, FString Message, bool bToConsole, bool bToScreen, FString Key)
{
	UUEUtil::Log.LogMessage(Level, Message, TEXT("Unreal Scenario"), bToConsole, bToScreen, Key);
}

