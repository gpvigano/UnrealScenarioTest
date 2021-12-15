// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EntityDataComponent.generated.h"


UCLASS(Blueprintable, ClassGroup=(UnrealScenario), meta=(BlueprintSpawnableComponent) )
/**
* Component that holds data for a scenario element.
*/
class UNREALSCENARIO_API UEntityDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	// Sets default values for this component's properties
	UEntityDataComponent();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Unique identifier of the entity.
		*/
		FString Identifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Entity type.
		*/
		FString Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Entity category.
		*/
		FString Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Entity description.
		*/
		FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Entity configuration text.
		*/
		FString Configuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Asset reference (to an item in a UAssetIndexComponent). 
		*/
		FString EntityAssetId;

	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "Unreal Scenario")
		/**
		Flag to keep track of changes to this entity.
		*/
		bool bModified = false;

protected:

	// Called when the game starts
	virtual void BeginPlay() override;

	
};
