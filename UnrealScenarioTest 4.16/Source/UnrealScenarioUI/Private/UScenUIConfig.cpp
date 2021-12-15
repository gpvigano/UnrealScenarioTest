// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "UScenUIConfig.h"

#include "UnrealScenarioUIActor.h"
#include "UEUtil.h"
#include "UScenUtil.h"


void UUScenUIConfig::UpdateRLConfiguration()
{
	if (!UnrealScenarioUIActor)
	{
		Init();
	}

	if (UnrealScenarioUIActor)
	{
		RLConfiguration = UnrealScenarioUIActor->GetRLConfiguration();
		RLConfigChanged = false;
	}
}


FRLConfig UUScenUIConfig::GetRLConfiguration()
{
	UpdateRLConfiguration();
	return RLConfiguration;
}


void UUScenUIConfig::SaveRLConfigurationChanges()
{
	if (!UnrealScenarioUIActor)
	{
		Init();
	}

	if (UnrealScenarioUIActor && RLConfigChanged)
	{
		// try to fix precision error in converting from double
		RLConfiguration.DiscountRate += 0.0000001f;
		UnrealScenarioUIActor->SetRLConfiguration(RLConfiguration);
		RLConfigChanged = false;
	}
}


