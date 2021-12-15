// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealScenarioUI.h"
#include "Modules/ModuleManager.h"

#include "UnrealScenarioUI.h"

DEFINE_LOG_CATEGORY(UnrealScenarioUI);

#define LOCTEXT_NAMESPACE "FUnrealScenarioUI"

void FUnrealScenarioUI::StartupModule()
{
	UE_LOG(UnrealScenarioUI, Display, TEXT("UnrealScenarioUI module has started."));
}

void FUnrealScenarioUI::ShutdownModule()
{
	UE_LOG(UnrealScenarioUI, Display, TEXT("UnrealScenarioUI module has shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUnrealScenarioUI, UnrealScenarioUI)
