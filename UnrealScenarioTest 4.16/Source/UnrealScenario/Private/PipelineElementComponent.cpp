// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "PipelineElementComponent.h"
#include "UScenUtil.h"


void UPipelineElementComponent::ExportConnectionPath()
{
	if (GetPipelineActor())
	{
		PipelineActor->ConnectionPath = ConnectionPath;
		PipelineActor->RebuildConnectionSections();
	}
}


void UPipelineElementComponent::ImportConnectionPath()
{
	if (GetPipelineActor())
	{
		ConnectionPath = PipelineActor->ConnectionPath;
	}
}


APipelineActor* UPipelineElementComponent::GetPipelineActor()
{
	if (::IsValid(PipelineActor))
	{
		return PipelineActor;
	}
	PipelineActor = nullptr;
	AActor* owner = GetOwner();
	if (owner == nullptr)
	{
		UUScenUtil::Log(Logger::ERROR, GetClass()->GetName() + TEXT(": PipelineActor not found"),true,true,"MissingPipelineActor");
		return nullptr;
	}
	PipelineActor = Cast<APipelineActor>(owner);
	return PipelineActor;
}


