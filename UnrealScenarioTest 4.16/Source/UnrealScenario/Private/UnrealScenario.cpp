// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealScenario.h"
#include "Modules/ModuleManager.h"

#include "UnrealScenario.h"

DEFINE_LOG_CATEGORY(UnrealScenario);

#define LOCTEXT_NAMESPACE "FUnrealScenario"

void FUnrealScenario::StartupModule()
{
	UE_LOG(UnrealScenario, Display, TEXT("UnrealScenario module has started."));
}

void FUnrealScenario::ShutdownModule()
{
	UE_LOG(UnrealScenario, Display, TEXT("UnrealScenario module has shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUnrealScenario, UnrealScenario)
