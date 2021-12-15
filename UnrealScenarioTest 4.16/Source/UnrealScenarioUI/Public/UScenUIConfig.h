// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"

#include "UScenUIBase.h"
#include "UnrealScenario.h"
#include "Runtime/UMG/Public/Components/ComboBox.h"



#include "UScenUIConfig.generated.h"

/**
 * User interface for Unreal Scenario test - Configuration.
 */
UCLASS(ClassGroup = UnrealScenarioUi)
class UNREALSCENARIOUI_API UUScenUIConfig : public UUScenUIBase
{
	GENERATED_BODY()
	
	

public:

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		void UpdateRLConfiguration();

	UFUNCTION(BlueprintPure, Category = "Unreal Scenario")
		FRLConfig GetRLConfiguration();

	UFUNCTION(BlueprintCallable, Category = "Unreal Scenario")
		void SaveRLConfigurationChanges();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		FRLConfig RLConfiguration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		bool RLConfigChanged = false;

	
	
	
};
