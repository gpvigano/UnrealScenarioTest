// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "UScenUIBase.h"

#include "UnrealScenarioUIActor.h"
#include "UScenUtil.h"

#include "Engine/World.h" // GetWorld()



AUnrealScenarioUIActor* UUScenUIBase::GetUnrealScenarioUIActor()
{
	if (!UnrealScenarioActor)
	{
		Init();
	}
	return UnrealScenarioUIActor;
}


AUnrealScenarioXpActor* UUScenUIBase::GetUnrealScenarioXpActor()
{
	if (!UnrealScenarioActor)
	{
		Init();
	}
	return UnrealScenarioXpActor;
}


AUnrealScenarioActor* UUScenUIBase::GetUnrealScenarioActor()
{
	if (!UnrealScenarioActor)
	{
		Init();
	}
	return UnrealScenarioActor;
}


void UUScenUIBase::Init()
{
	if (!UnrealScenarioActor)
	{
		UnrealScenarioActor = Cast<AUnrealScenarioActor>(UUScenUtil::FindUnrealScenario(GetWorld()));
		UnrealScenarioXpActor = Cast<AUnrealScenarioXpActor>(UnrealScenarioActor);
		UnrealScenarioUIActor = Cast<AUnrealScenarioUIActor>(UnrealScenarioXpActor);

		if (!UnrealScenarioActor)
		{
			UUScenUtil::Log(Logger::FATAL, "No Unreal Scenario Actor found!", true, true, "UI_MISSING_UnrealScenarioActor");
		}
		/*
		TArray<AActor*> actors = GetWorld()->GetLevel(0)->Actors;
		for (AActor* actor : actors)
		{
		UnrealScenarioUIActor = Cast<AUnrealScenarioUIActor>(actor);
		if (UnrealScenarioUIActor)
		{
		break;
		}
		}
		for (TActorIterator<AUnrealScenarioUIActor> actorItr(GetWorld()); actorItr; ++actorItr)
		{
		if (actorItr)
		{
		UnrealScenarioUIActor = *actorItr;
		break;
		}
		}
		*/
	}
}



