// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnrealScenarioUIActor.h"
#include "UnrealScenario.h"


#include "UScenUIBase.generated.h"

UCLASS(ClassGroup = UnrealScenarioUi)
/**
 * Unreal Scenario Test User Interface base widget class.
 */
class UNREALSCENARIOUI_API UUScenUIBase : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		AUnrealScenarioUIActor* GetUnrealScenarioUIActor();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		AUnrealScenarioXpActor* GetUnrealScenarioXpActor();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		AUnrealScenarioActor* GetUnrealScenarioActor();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Unreal Scenario")
		void UpdateUI();

protected:

	AUnrealScenarioActor* UnrealScenarioActor;

	AUnrealScenarioXpActor* UnrealScenarioXpActor;

	AUnrealScenarioUIActor* UnrealScenarioUIActor;
	
	virtual void Init();
};
