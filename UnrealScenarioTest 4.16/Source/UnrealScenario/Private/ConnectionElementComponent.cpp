// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "ConnectionElementComponent.h"
#include "UScenUtil.h"


UConnectionElementComponent::UConnectionElementComponent()
{
}


void UConnectionElementComponent::ExportConnectionPath()
{
	UUScenUtil::Log(Logger::ERROR,GetClass()->GetName() + TEXT("::ExportConnectionPath() not implemented"));
}


void UConnectionElementComponent::ImportConnectionPath()
{
	UUScenUtil::Log(Logger::ERROR,GetClass()->GetName() + TEXT("::ImportConnectionPath() not implemented"));
}


float UConnectionElementComponent::ComputeConnectionPathLength()
{
	if (ConnectionPath.Num() > 1)
	{
		FVector prevPos;
		float dist = 0.0f;
		for (int32 i = 0; i < ConnectionPath.Num(); i++)
		{
			const FConnectionAnchor& anchor = ConnectionPath[i];
			FVector anchorPos = UUScenUtil::GetAnchorWorldPosition(anchor);
			if (i > 0)
			{
				dist += FVector::Distance(anchorPos, prevPos);
			}
			prevPos = anchorPos;
		}
		return dist;
	}
	return 0.0f;
}


