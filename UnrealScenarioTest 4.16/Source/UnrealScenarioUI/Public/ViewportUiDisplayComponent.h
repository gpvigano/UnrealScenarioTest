// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UScenUIBase.h"



#include "ViewportUiDisplayComponent.generated.h"


UCLASS(ClassGroup = UnrealScenario, meta=(BlueprintSpawnableComponent) )
class UNREALSCENARIOUI_API UViewportUiDisplayComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario|User Interface")
	/**
	Widget blueprint for the main control interface
	*/
		TSubclassOf<UUScenUIBase> UiWidgetBlueprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario|User Interface")
	/**
	Widget blueprint for the main control interface
	*/
		bool bStartWithUiVisible = true;



	// Sets default values for this component's properties
	UViewportUiDisplayComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


protected:

	UPROPERTY()
		UUScenUIBase* UiWidget = nullptr;


		
	// Called when the game starts
	virtual void BeginPlay() override;
	
};
