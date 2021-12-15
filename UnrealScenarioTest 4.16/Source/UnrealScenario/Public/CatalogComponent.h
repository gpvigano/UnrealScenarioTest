// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnrealScenario.h"



#include "CatalogComponent.generated.h"


UCLASS(ClassGroup = UnrealScenario)
/**
* Unreal Scenario catalog component.
*/
class UNREALSCENARIO_API UCatalogComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCatalogComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Name of the catalog.
		*/
		FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Path to the file where the catalog is saved or loaded.
		*/
		FString FileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		List of items included in this catalog.
		*/
		TArray<FCatalogItem> Items;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal Scenario")
		/**
		Catalog categories with their abbreviations, used to group items.
		*/
		TMap<FString, FString> CategoryAbbreviations;



	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
		Load the catalog from the path set in FileName.
		*/
		void LoadCatalog();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Unreal Scenario")
		/**
		Save the catalog to the path set in FileName.
		*/
		void SaveCatalog();

};
