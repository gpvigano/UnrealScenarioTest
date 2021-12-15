// UnrealScenarioUI - (c)2020-2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"


DECLARE_LOG_CATEGORY_EXTERN(UnrealScenarioUI, All, All);


class FUnrealScenarioUI : public IModuleInterface
{
public:

	/* This will get called when the editor loads the module */
	virtual void StartupModule() override;

	/* This will get called when the editor unloads the module */
	virtual void ShutdownModule() override;
};

